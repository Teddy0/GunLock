// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GunLockGameModeTeam.generated.h"

UCLASS(minimalapi)
class AGunLockGameModeTeam : public AGunLockGameMode
{
	GENERATED_UCLASS_BODY()

	int32 ChooseTeam();

	virtual void Reset() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual class AActor* ChoosePlayerStart(AController* Player) override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual void InitNewPlayer(class AController* NewPlayer, const TSharedPtr<FUniqueNetId>& UniqueId, const FString& Options) override;
	virtual void AddInactivePlayer(APlayerState* PlayerState, APlayerController* PC) override;

	virtual bool ReadyToStartMatch() override;
	virtual bool ReadyToEndMatch() override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;

	virtual void PlayerKilled(AController* Victim, AController* Killer);

	/** How long a round lasts for (in minutes) */
	UPROPERTY(BlueprintReadWrite, Category = Gameplay)
	float RoundTime;

	/** How long the post round lasts for (in seconds) */
	UPROPERTY(BlueprintReadWrite, Category = Gameplay)
	float PostRoundTime;

private:
	UPROPERTY()
	TArray<class AGunLockPlayerController*> BlueTeamPlayers;

	UPROPERTY()
	TArray<class AGunLockPlayerController*> RedTeamPlayers;

	UPROPERTY()
	TArray<class AGunLockPlayerController*> PendingPlayers;

	UPROPERTY()
	float RoundStartTime;

	UPROPERTY()
	int32 WinningTeam;
};



