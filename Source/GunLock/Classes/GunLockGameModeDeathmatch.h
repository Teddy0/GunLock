// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GunLockGameModeDeathmatch.generated.h"

UCLASS(minimalapi)
class AGunLockGameModeDeathmatch : public AGunLockGameMode
{
	GENERATED_UCLASS_BODY()

	virtual void Tick(float DeltaSeconds) override;

	virtual class AActor* ChoosePlayerStart(AController* Player) override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

	virtual void PlayerKilled(AController* Victim, AController* Killer);
};