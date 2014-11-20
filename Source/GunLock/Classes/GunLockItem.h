// Teddy0@gmail.com
#pragma once

#include "GunLockItem.generated.h"

UCLASS(config = Game)
class AGunLockItem : public AActor
{
	GENERATED_UCLASS_BODY()

	virtual bool RightItemHand() { return false; }

	virtual void ItemPickedup(AGunLockCharacter* NewOwner);

	virtual void GetHandStates(int32& RightHandState, int32& LeftHandState)
	{
	}

	virtual void DropItem();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particles)
	class UParticleSystem* DropItemEffect;

	UPROPERTY()
	AGunLockItemSpawnPoint* SpawnPoint;

	UPROPERTY()
	bool bItemDropped;
};

