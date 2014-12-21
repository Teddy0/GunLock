// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GunLock.h"
#include "GunLockGameModeTeam.h"
#include "GunLockHUD.h"

AGunLockGameModeTeam::AGunLockGameModeTeam(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	bStartPlayersAsSpectators = true;

	RoundTime = 5.f;
	PostRoundTime = 15.f;
}

void AGunLockGameModeTeam::Reset()
{
	Super::Reset();

	//Reset spawn point tracking
	EmptyItemSpawnPoints.Append(ItemSpawnPoints);
	ItemSpawnPoints.Empty();
	WinningTeam = 0;
}

bool AGunLockGameModeTeam::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

AActor* AGunLockGameModeTeam::ChoosePlayerStart(AController* Player)
{
	// Find a player start that matches our team
	APawn* PawnToFit = DefaultPawnClass ? DefaultPawnClass->GetDefaultObject<APawn>() : NULL;
	AGunLockPlayerController* PC = Cast<AGunLockPlayerController>(Player);
	check(PC);
	
	FName TeamTag;
	if (BlueTeamPlayers.Contains(PC))
		TeamTag = TEXT("Blue");
	else if (RedTeamPlayers.Contains(PC))
		TeamTag = TEXT("Red");

	APlayerStart* BestPlayerStart = NULL;
	for (int32 PlayerStartIndex = 0; PlayerStartIndex < PlayerStarts.Num(); ++PlayerStartIndex)
	{
		APlayerStart* PlayerStart = PlayerStarts[PlayerStartIndex];
		if (PlayerStart != NULL)
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			if (Cast<APlayerStartPIE>(PlayerStart) != NULL)
				return PlayerStart;

			// Find a player start that matches our team
			if (PlayerStart->PlayerStartTag == TeamTag)
			{
				// Check if it's already occupied
				FVector ActorLocation = PlayerStart->GetActorLocation();
				const FRotator ActorRotation = PlayerStart->GetActorRotation();
				if (!GetWorld()->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
				{
					BestPlayerStart = PlayerStart;
					break;
				}
			}
		}
	}
	//If we couldn't find any suitable spawns, pick a random one!
	if (BestPlayerStart == NULL && PlayerStarts.Num() > 0)
	{
		return PlayerStarts[FMath::RandRange(0, PlayerStarts.Num() - 1)];
	}
	return BestPlayerStart;
}

void AGunLockGameModeTeam::InitNewPlayer(AController* NewPlayer, const TSharedPtr<FUniqueNetId>& UniqueId, const FString& Options)
{
	AGunLockPlayerController* PC = Cast<AGunLockPlayerController>(NewPlayer);
	check(PC);
	PendingPlayers.Add(PC);
}

void AGunLockGameModeTeam::AddInactivePlayer(APlayerState* PlayerState, APlayerController* PC)
{
	AGunLockPlayerController* GunLockPC = Cast<AGunLockPlayerController>(PC);
	check(GunLockPC);
	PendingPlayers.Remove(GunLockPC);
	BlueTeamPlayers.Remove(GunLockPC);
	RedTeamPlayers.Remove(GunLockPC);
}

int32 AGunLockGameModeTeam::ChooseTeam()
{
	int32 TeamId = 1 + FMath::RandHelper(2);
	if (BlueTeamPlayers.Num() < RedTeamPlayers.Num())
		TeamId = 1;
	else if (RedTeamPlayers.Num() < BlueTeamPlayers.Num())
		TeamId = 2;

	return TeamId;
}

bool AGunLockGameModeTeam::ReadyToStartMatch()
{
	// By default start when we have > 0 players
	if (GetMatchState() == MatchState::WaitingToStart)
	{
		if (NumPlayers + NumBots > 0)
		{
			return true;
		}
	}
	return false;
}

bool AGunLockGameModeTeam::ReadyToEndMatch()
{
	//Check if all players on one team are dead
	int32 BlueLivingPlayers = 0;
	for (int32 i = 0; i < BlueTeamPlayers.Num(); i++)
	{
		AGunLockCharacter* Player = Cast<AGunLockCharacter>(BlueTeamPlayers[i]->GetPawn());
		if (Player && !Player->bIsDead)
			BlueLivingPlayers++;
	}
	int32 RedLivingPlayers = 0;
	for (int32 i = 0; i < RedTeamPlayers.Num(); i++)
	{
		AGunLockCharacter* Player = Cast<AGunLockCharacter>(RedTeamPlayers[i]->GetPawn());
		if (Player && !Player->bIsDead)
			RedLivingPlayers++;
	}

	//If there's only 1 player in the server, end the round when someone else joins
	if (BlueTeamPlayers.Num() + RedTeamPlayers.Num() == 1)
	{
		return PendingPlayers.Num() > 0;
	}
	else if (BlueLivingPlayers == 0 || RedLivingPlayers == 0)
	{
		WinningTeam = (BlueLivingPlayers == 0) ? 2 : 1;
		return true;
	}

	//Check the round timer
	return RoundTime > 0.f && FPlatformTime::Seconds() - RoundStartTime >= RoundTime * 60.f;
}

void AGunLockGameModeTeam::HandleMatchHasStarted()
{
	//Reset the world
	{
		//Destroy all pawns
		for (FConstPawnIterator Iterator = GetWorld()->GetPawnIterator(); Iterator; ++Iterator)
		{
			APawn* Pawn = *Iterator;
			GetWorld()->RemovePawn(Pawn);
			Pawn->Destroy();
			--Iterator;
		}

		//Clean up previous round's items
		for (TObjectIterator<AGunLockItem> It; It; ++It)
		{
			AGunLockItem* TestItem = *It;
			TestItem->ItemDestroyedEvent();
			TestItem->Destroy();
		}

		// Reset all actors (except controllers, the GameMode, and any other actors specified by ShouldReset())
		for (FActorIterator It(GetWorld()); It; ++It)
		{
			AActor* A = *It;
			if (A && !A->IsPendingKill() && A != this && !A->IsA<AController>() && ShouldReset(A))
			{
				A->Reset();
			}
		}

		// reset the GameMode
		Reset();

		// notify the clients to reset their worlds
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			AGunLockPlayerController* PC = Cast<AGunLockPlayerController>(*Iterator);
			if (PC)
				PC->ResetClient();
		}

		// Notify the level script that the level has been reset
		ALevelScriptActor* LevelScript = GetWorld()->GetLevelScriptActor();
		if (LevelScript)
		{
			LevelScript->LevelReset();
		}
	}

	//Add any pending players to teams
	for (int32 i = 0; i < PendingPlayers.Num(); i++)
	{
		int32 TeamId = ChooseTeam();
		if (TeamId == 1)
			BlueTeamPlayers.Add(PendingPlayers[i]);
		else
			RedTeamPlayers.Add(PendingPlayers[i]);
	}
	PendingPlayers.Empty();

	//Check teams are even
	if (BlueTeamPlayers.Num() > RedTeamPlayers.Num())
	{
		RedTeamPlayers.Push(BlueTeamPlayers.Pop());
	}
	else if (RedTeamPlayers.Num() > BlueTeamPlayers.Num())
	{
		BlueTeamPlayers.Push(RedTeamPlayers.Pop());
	}

	//Ensure all the playerstates are up to date
	for (int32 i = 0; i < BlueTeamPlayers.Num(); i++)
	{
		AGunLockPlayerState* PlayerState = Cast<AGunLockPlayerState>(BlueTeamPlayers[i]->PlayerState);
		check(PlayerState);
		PlayerState->SetTeamNum(1);
	}
	for (int32 i = 0; i < RedTeamPlayers.Num(); i++)
	{
		AGunLockPlayerState* PlayerState = Cast<AGunLockPlayerState>(RedTeamPlayers[i]->PlayerState);
		check(PlayerState);
		PlayerState->SetTeamNum(2);
	}

	//Parent function spawns the players
	Super::HandleMatchHasStarted();

	RoundStartTime = FPlatformTime::Seconds();

	int32 RoundPlayers = BlueTeamPlayers.Num() + RedTeamPlayers.Num();

	//Spawn a weapon for each player
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (EmptyItemSpawnPoints.Num() == 0 || WeaponBlueprints.Num() == 0)
			break;

		APlayerController* PlayerController = *Iterator;
		AGunLockCharacter* Pawn = Cast<AGunLockCharacter>(PlayerController->GetPawn());

		//Spawn a weapon for each player, as close to them as possible
		if (Pawn)
		{
			// Try to spawn the weapon near a player with no weapon
			AGunLockItemSpawnPoint* ItemSpawnPoint = GetClosestSpawnPoint(Pawn->GetActorLocation());

			// If we can't find one nearby, pick a random empty one
			if (!ItemSpawnPoint)
				ItemSpawnPoint = GetRandomSpawnPoint();

			// If that fails, stop trying to spawn weapons
			if (!ItemSpawnPoint)
				break;

			FVector SpawnLocation = ItemSpawnPoint->GetActorLocation() + FVector(FMath::FRandRange(-8.f, 8.f), FMath::FRandRange(-8.f, 8.f), 5.f);
			FRotator SpawnRotation = ItemSpawnPoint->GetActorRotation() + FRotator(90.f, 0.f, 0.f);

			//Spawn the item
			int32 RandomWeapon = FMath::RandHelper(WeaponBlueprints.Num());
			UBlueprint* SpawnBlueprint = WeaponBlueprints[RandomWeapon];
			AGunLockItem* SpawnedItem = (AGunLockItem*)GetWorld()->SpawnActor(SpawnBlueprint->GeneratedClass, &SpawnLocation, &SpawnRotation);
			SpawnedItem->SpawnPoint = ItemSpawnPoint;
			SpawnedWeaponCount++;
			Pawn->bHasSpawnedGunForPlayer = true;
		}

		//Spawn a magazine for each player
		if (Pawn)
		{
			AGunLockItemSpawnPoint* ItemSpawnPoint = GetClosestSpawnPoint(Pawn->GetActorLocation());
			if (ItemSpawnPoint)
			{
				FVector SpawnLocation = ItemSpawnPoint->GetActorLocation() + FVector(FMath::FRandRange(-8.f, 8.f), FMath::FRandRange(-8.f, 8.f), 0);
				FRotator SpawnRotation = ItemSpawnPoint->GetActorRotation();

				//Spawn the item
				int32 RandomMagazine = FMath::RandHelper(MagazineBlueprints.Num());
				UBlueprint* SpawnBlueprint = MagazineBlueprints[RandomMagazine];
				AGunLockItem* SpawnedItem = (AGunLockItem*)GetWorld()->SpawnActor(SpawnBlueprint->GeneratedClass, &SpawnLocation, &SpawnRotation);
				SpawnedItem->SpawnPoint = ItemSpawnPoint;
			}
		}
	}

	for (int32 i=0; i < 26; i++)
	{
		if (EmptyItemSpawnPoints.Num() == 0 || AmmoBlueprint == NULL)
			break;

		AGunLockItemSpawnPoint* ItemSpawnPoint = GetRandomSpawnPoint();
		FVector SpawnLocation = ItemSpawnPoint->GetActorLocation() + FVector(FMath::FRandRange(-8.f, 8.f), FMath::FRandRange(-8.f, 8.f), 1.f);
		FRotator SpawnRotation = ItemSpawnPoint->GetActorRotation();

		//Spawn the item
		AGunLockItem* SpawnedItem = (AGunLockItem*)GetWorld()->SpawnActor(AmmoBlueprint->GeneratedClass, &SpawnLocation, &SpawnRotation);
		SpawnedItem->SpawnPoint = ItemSpawnPoint;
	}
}

void AGunLockGameModeTeam::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	//Begin PostMatch countdown
	RoundStartTime = FPlatformTime::Seconds();

	//Force all players to spectate

	// notify the clients to reset their worlds
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AGunLockPlayerController* PC = Cast<AGunLockPlayerController>(*Iterator);
		if (PC)
			PC->RoundWinSound(WinningTeam);
	}
}

void AGunLockGameModeTeam::PlayerKilled(AController* Victim, AController* Killer)
{
	if (Victim != Killer && Killer && Killer->PlayerState)
	{
		AGunLockPlayerState* VictimState = Cast<AGunLockPlayerState>(Victim->PlayerState);
		AGunLockPlayerState* KillerState = Cast<AGunLockPlayerState>(Killer->PlayerState);
		check(VictimState && KillerState);
		if (VictimState->GetTeamNum() == KillerState->GetTeamNum())
			Killer->PlayerState->Score -= 2.0f;
		else
			Killer->PlayerState->Score += 1.0f;
	}

	Victim->ChangeState(NAME_Spectating);
}

void AGunLockGameModeTeam::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Post round match timer
	if (GetMatchState() == MatchState::WaitingPostMatch)
	{
		if (FPlatformTime::Seconds() - RoundStartTime >= PostRoundTime)
		{
			SetMatchState(MatchState::WaitingToStart);
		}
	}
}