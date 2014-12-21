// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GunLock.h"
#include "GunLockGameModeDeathmatch.h"

AGunLockGameModeDeathmatch::AGunLockGameModeDeathmatch(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
}

bool AGunLockGameModeDeathmatch::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

AActor* AGunLockGameModeDeathmatch::ChoosePlayerStart(AController* Player)
{
	// Find a player start that is furthest from other players
	APawn* PawnToFit = DefaultPawnClass ? DefaultPawnClass->GetDefaultObject<APawn>() : NULL;
	APlayerStart* BestPlayerStart = NULL;
	float MaxPlayerDistance = 0.0f;

	for (int32 PlayerStartIndex = 0; PlayerStartIndex < PlayerStarts.Num(); ++PlayerStartIndex)
	{
		APlayerStart* PlayerStart = PlayerStarts[PlayerStartIndex];
		if (PlayerStart != NULL)
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			if (Cast<APlayerStartPIE>(PlayerStart) != NULL)
				return PlayerStart;

			// Find the minimum distance to other players
			float MinPlayerDistance = HALF_WORLD_MAX;
			FVector SpawnLocation = PlayerStart->GetActorLocation();
			const FRotator SpawnRotation = PlayerStart->GetActorRotation();
			for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{
				APlayerController* PlayerController = *Iterator;
				if (Player != PlayerController && PlayerController->GetPawn())
				{
					MinPlayerDistance = FMath::Min(MinPlayerDistance, (SpawnLocation - PlayerController->GetPawn()->GetActorLocation()).Size());
				}
			}

			// Choose the spawn that is furtherest from all other players, to give them a fighting chance!
			if (MinPlayerDistance > MaxPlayerDistance)
			{
				MaxPlayerDistance = MinPlayerDistance;
				BestPlayerStart = PlayerStart;
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

void AGunLockGameModeDeathmatch::PlayerKilled(AController* Victim, AController* Killer)
{
	Super::PlayerKilled(Victim, Killer);

	//Respawn the player after 10 seconds
	APlayerController* GunLockVictim = Cast<APlayerController>(Victim);
	GetWorldTimerManager().SetTimer(GunLockVictim, &APlayerController::ServerRestartPlayer, 10.f);
}

void AGunLockGameModeDeathmatch::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	int32 LivingPlayers = 0;
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = *Iterator;
			AGunLockCharacter* Pawn = Cast<AGunLockCharacter>(PlayerController->GetPawn());
			if (Pawn && !Pawn->bIsDead)
				LivingPlayers++;
		}
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (EmptyItemSpawnPoints.Num() == 0 || WeaponBlueprints.Num() == 0 || SpawnedWeaponCount >= LivingPlayers)
			break;

		APlayerController* PlayerController = *Iterator;
		AGunLockCharacter* Pawn = Cast<AGunLockCharacter>(PlayerController->GetPawn());

		//Spawn a weapon for each player, as close to them as possible
		if (Pawn && !Pawn->bHasSpawnedGunForPlayer)
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
	}

	for (; SpawnedMagazineCount < LivingPlayers;)
	{
		if (EmptyItemSpawnPoints.Num() == 0 || MagazineBlueprints.Num() == 0)
			break;

		AGunLockItemSpawnPoint* ItemSpawnPoint = GetRandomSpawnPoint();
		FVector SpawnLocation = ItemSpawnPoint->GetActorLocation() + FVector(FMath::FRandRange(-8.f, 8.f), FMath::FRandRange(-8.f, 8.f), 0);
		FRotator SpawnRotation = ItemSpawnPoint->GetActorRotation();

		//Spawn the item
		int32 RandomWeapon = FMath::RandHelper(MagazineBlueprints.Num());
		UBlueprint* SpawnBlueprint = MagazineBlueprints[RandomWeapon];
		AGunLockItem* SpawnedItem = (AGunLockItem*)GetWorld()->SpawnActor(SpawnBlueprint->GeneratedClass, &SpawnLocation, &SpawnRotation);
		SpawnedItem->SpawnPoint = ItemSpawnPoint;
		SpawnedMagazineCount++;
	}

	for (; SpawnedAmmoCount < LivingPlayers * 4;)
	{
		if (EmptyItemSpawnPoints.Num() == 0 || AmmoBlueprint == NULL)
			break;

		AGunLockItemSpawnPoint* ItemSpawnPoint = GetRandomSpawnPoint();
		FVector SpawnLocation = ItemSpawnPoint->GetActorLocation() + FVector(FMath::FRandRange(-8.f, 8.f), FMath::FRandRange(-8.f, 8.f), 1.f);
		FRotator SpawnRotation = ItemSpawnPoint->GetActorRotation();

		//Spawn the item
		AGunLockItem* SpawnedItem = (AGunLockItem*)GetWorld()->SpawnActor(AmmoBlueprint->GeneratedClass, &SpawnLocation, &SpawnRotation);
		SpawnedItem->SpawnPoint = ItemSpawnPoint;
		SpawnedAmmoCount++;
	}
}