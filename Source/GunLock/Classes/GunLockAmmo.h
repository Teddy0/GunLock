// Teddy0@gmail.com
#pragma once

#include "GunLockItem.h"
#include "GunLockAmmo.generated.h"

UCLASS(config = Game)
class AGunLockAmmo : public AGunLockItem
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TSubobjectPtr<class UStaticMeshComponent> AmmoMesh;

	UPROPERTY(VisibleAnywhere, Category = General)
	int32 MaxRounds;

	UFUNCTION()
	void OnRep_RoundsUpdated();
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_RoundsUpdated)
	int32 Rounds;

	UPROPERTY()
	TArray<class UMaterialInstanceDynamic*> BulletMaterials;

	UPROPERTY()
	bool bPickupInRightHand;
	virtual void AllowRightHandPickup(bool bAllowRightHand) { bPickupInRightHand = bAllowRightHand; }

	virtual void BeginPlay() override;
	virtual void ItemPickedup(AGunLockCharacter* NewOwner);
	virtual void GetHandStates(int32& RightHandState, int32& LeftHandState);
	virtual bool RightItemHand() { return bPickupInRightHand; }

	void AmmoPickedup(AGunLockCharacter* NewOwner);
	void UpdateBulletMaterials();
};

