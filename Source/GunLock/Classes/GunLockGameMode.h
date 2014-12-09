// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GunLockGameMode.generated.h"

UCLASS(minimalapi)
class AGunLockGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

	virtual void Tick(float DeltaSeconds) override;

	virtual class AActor* ChoosePlayerStart(AController* Player) override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

	class AGunLockItemSpawnPoint* GetRandomSpawnPoint();
	class AGunLockItemSpawnPoint* GetClosestSpawnPoint(FVector Point);

	UPROPERTY()
	TArray<class AGunLockItemSpawnPoint*> EmptyItemSpawnPoints;

	UPROPERTY()
	TArray<class AGunLockItemSpawnPoint*> ItemSpawnPoints;

	UPROPERTY()
	int32 SpawnedWeaponCount;

	UPROPERTY()
	int32 SpawnedMagazineCount;

	UPROPERTY()
	int32 SpawnedAmmoCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=gameplay)
	TArray<class UBlueprint*> WeaponBlueprints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = gameplay)
	TArray<class UBlueprint*> MagazineBlueprints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = gameplay)
	class UBlueprint* AmmoBlueprint;
};



