// Teddy0@gmail.com
#pragma once

#include "GunLockItemSpawnPoint.generated.h"

UCLASS(config = Game)
class AGunLockItemSpawnPoint : public AActor
{
	GENERATED_UCLASS_BODY()

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
};

