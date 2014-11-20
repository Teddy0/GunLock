// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GunLock.h"
#include "GunLockGameMode.h"
#include "GunLockHUD.h"

AGunLockGameMode::AGunLockGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FObjectFinder<UBlueprint> PlayerPawnObject(TEXT("/Game/Blueprints/VRPlayer"));
	if (PlayerPawnObject.Object != NULL)
	{
		DefaultPawnClass = (UClass*)PlayerPawnObject.Object->GeneratedClass;
	}

	// use our custom HUD class
	HUDClass = AGunLockHUD::StaticClass();
}

bool AGunLockGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

AActor* AGunLockGameMode::ChoosePlayerStart(AController* Player)
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

AGunLockItemSpawnPoint* AGunLockGameMode::GetRandomSpawnPoint()
{
	int32 RandomSpawnPoint = FMath::RandHelper(EmptyItemSpawnPoints.Num());
	AGunLockItemSpawnPoint* ItemSpawnPoint = EmptyItemSpawnPoints[RandomSpawnPoint];

	//Move the spawn point to the other list
	EmptyItemSpawnPoints.RemoveAtSwap(RandomSpawnPoint);
	ItemSpawnPoints.Add(ItemSpawnPoint);

	return ItemSpawnPoint;
}

AGunLockItemSpawnPoint* AGunLockGameMode::GetClosestSpawnPoint(FVector Point)
{
	float ClosestSpawn = HALF_WORLD_MAX;
	int32 ClosestSpawnIdx = 0;
	for (int32 i = 0; i < EmptyItemSpawnPoints.Num(); i++)
	{
		float SpawnDistance = (Point - EmptyItemSpawnPoints[i]->GetActorLocation()).Size();
		if (SpawnDistance < ClosestSpawn)
		{
			ClosestSpawn = SpawnDistance;
			ClosestSpawnIdx = i;
		}
	}

	//Move the spawn point to the other list
	AGunLockItemSpawnPoint* ItemSpawnPoint = EmptyItemSpawnPoints[ClosestSpawnIdx];
	EmptyItemSpawnPoints.RemoveAtSwap(ClosestSpawnIdx);
	ItemSpawnPoints.Add(ItemSpawnPoint);

	return ItemSpawnPoint;
}

void AGunLockGameMode::Tick(float DeltaSeconds)
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

	FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator();
	for (; SpawnedWeaponCount < LivingPlayers;)
	{
		if (EmptyItemSpawnPoints.Num() == 0 || WeaponBlueprints.Num() == 0)
			break;

		// Try to spawn the weapon near a player with no weapon
		AGunLockItemSpawnPoint* ItemSpawnPoint = NULL;
		for (; Iterator; ++Iterator)
		{
			APlayerController* PlayerController = *Iterator;
			AGunLockCharacter* Pawn = Cast<AGunLockCharacter>(PlayerController->GetPawn());
			if (Pawn && Pawn->RightHandItem == NULL)
			{
				ItemSpawnPoint = GetClosestSpawnPoint(Pawn->GetActorLocation());
				break;
			}
		}
		if (!ItemSpawnPoint)
			ItemSpawnPoint = GetRandomSpawnPoint();

		FVector SpawnLocation = ItemSpawnPoint->GetActorLocation() + FVector(FMath::FRandRange(-8.f, 8.f), FMath::FRandRange(-8.f, 8.f), 5.f);
		FRotator SpawnRotation = ItemSpawnPoint->GetActorRotation() + FRotator(90.f, 0.f, 0.f);

		//Spawn the item
		int32 RandomWeapon = FMath::RandHelper(WeaponBlueprints.Num());
		UBlueprint* SpawnBlueprint = WeaponBlueprints[RandomWeapon];
		AGunLockItem* SpawnedItem = (AGunLockItem*)GetWorld()->SpawnActor(SpawnBlueprint->GeneratedClass, &SpawnLocation, &SpawnRotation);
		SpawnedItem->SpawnPoint = ItemSpawnPoint;
		SpawnedWeaponCount++;
	}

	for (; SpawnedMagazineCount < LivingPlayers*2;)
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
}