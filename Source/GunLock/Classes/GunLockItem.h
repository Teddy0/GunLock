// Teddy0@gmail.com
#pragma once

#include "GunLockItem.generated.h"

UCLASS(config = Game)
class AGunLockItem : public AActor
{
	GENERATED_UCLASS_BODY()

	virtual bool RightItemHand() { return false; }
	virtual void AllowRightHandPickup(bool bAllowRightHand) {};
	virtual bool CanPickupItem();

	virtual void ItemPickedup(AGunLockCharacter* NewOwner);
	virtual void NotifyOwnerDied();

	UFUNCTION(BlueprintImplementableEvent)
	void ItemPickedupEvent();

	UFUNCTION(BlueprintImplementableEvent)
	void ItemDestroyedEvent();

	virtual void GetHandStates(int32& RightHandState, int32& LeftHandState)
	{
	}

	UFUNCTION(NetMulticast, unreliable)
	virtual void NetMulticast_PlaySound(class USoundCue* Sound, bool bFollow);

	virtual void DropItem();
	virtual bool ShouldDestroyOnDrop() { return false;  }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particles)
	class UParticleSystem* DropItemEffect;

	UPROPERTY()
	AGunLockItemSpawnPoint* SpawnPoint;
};

