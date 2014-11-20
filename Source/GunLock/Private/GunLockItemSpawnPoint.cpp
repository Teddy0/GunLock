// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockItemSpawnPoint.h"

AGunLockItemSpawnPoint::AGunLockItemSpawnPoint(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
}

void AGunLockItemSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	AGunLockGameMode* const GameMode = GetWorld() ? Cast<AGunLockGameMode>(GetWorld()->GetAuthGameMode()) : NULL;
	if (GameMode)
	{
		GameMode->EmptyItemSpawnPoints.Add(this);
	}
}

void AGunLockItemSpawnPoint::BeginDestroy()
{
	Super::BeginDestroy();

	AGunLockGameMode* const GameMode = GetWorld() ? Cast<AGunLockGameMode>(GetWorld()->GetAuthGameMode()) : NULL;
	if (GameMode)
	{
		GameMode->EmptyItemSpawnPoints.Remove(this);
		GameMode->ItemSpawnPoints.Remove(this);
	}
}