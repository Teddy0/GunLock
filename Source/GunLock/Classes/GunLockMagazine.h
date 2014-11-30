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

	UPROPERTY(Replicated, VisibleAnywhere, Category = General)
	int32 Rounds;

	UPROPERTY()
	TArray<class UMaterialInstanceDynamic*> BulletMaterials;

	virtual void BeginPlay() override;
	virtual void GetHandStates(int32& RightHandState, int32& LeftHandState);
	virtual void ItemPickedup(AGunLockCharacter* NewOwner);
	virtual bool CanPickupItem();
	virtual void DropItem();
	virtual bool ShouldDestroyOnDrop() { return Rounds == 0; }

	void AttachToGun(class AGunLockWeapon* NewWeapon);

	void UpdateBulletMaterials();
};

