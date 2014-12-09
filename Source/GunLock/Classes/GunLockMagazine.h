// Teddy0@gmail.com
#pragma once

#include "GunLockItem.h"
#include "GunLockMagazine.generated.h"

UCLASS(config = Game)
class AGunLockMagazine : public AGunLockItem
{
	GENERATED_UCLASS_BODY()

	/** Gun mesh: Represents the player's Magazine */
	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TSubobjectPtr<class UStaticMeshComponent> MagazineMesh;

	UPROPERTY(VisibleAnywhere, Category=General)
	int32 MaxRounds;

	UFUNCTION(reliable, client)
	void ClientUpdateRounds(int32 InRounds);
	UFUNCTION()
	void OnRep_RoundsUpdated();
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_RoundsUpdated)
	int32 Rounds;

	UPROPERTY()
	TArray<class UMaterialInstanceDynamic*> BulletMaterials;

	virtual void BeginPlay() override;
	virtual void GetHandStates(int32& RightHandState, int32& LeftHandState);
	virtual void ItemPickedup(AGunLockCharacter* NewOwner);
	virtual bool CanPickupItem();
	virtual void DropItem();

	void AttachToGun(class AGunLockWeapon* NewWeapon);

	void UpdateBulletMaterials();
};

