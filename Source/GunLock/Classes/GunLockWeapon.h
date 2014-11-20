// Teddy0@gmail.com
#pragma once

#include "GunLockItem.h"
#include "GunLockWeapon.generated.h"

USTRUCT()
struct FShotInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	FVector HitLocation;

	UPROPERTY()
	FVector HitNormal;

	UPROPERTY()
	int32 SurfaceType;
};

UCLASS(config = Game)
class AGunLockWeapon : public AGunLockItem
{
	GENERATED_UCLASS_BODY()

	/** Gun mesh: Represents the player's weapon */
	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TSubobjectPtr<class USkeletalMeshComponent> GunMesh;

	/** Particle system for firing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particles)
	class UParticleSystem* MuzzleEffect;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particles)
	class UParticleSystem* CasingEffect;

	/** Sound nodes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Audio)
	class USoundCue* GunshotSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Audio)
	class USoundCue* CasingSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Audio)
	class USoundCue* SlideReleaseSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Audio)
	class USoundCue* MagazineOutSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Audio)
	class USoundCue* MagazineInSound;

	/** Bullet impact sounds/FX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Audio)
	class USoundCue* ConcreteImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Audio)
	class USoundCue* MetalImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Audio)
	class USoundCue* MetalSolidImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Audio)
	class USoundCue* WoodImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Audio)
	class USoundCue* FleshImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particles)
	class UParticleSystem* ConcreteImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particles)
	class UParticleSystem* MetalImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particles)
	class UParticleSystem* WoodImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particles)
	class UParticleSystem* FleshImpactEffect;

	/* Ammo and magazine values */
	UPROPERTY()
	class AGunLockMagazine* AttachedMagazine;

	UPROPERTY()
	bool RoundChambered;

	UPROPERTY()
	bool SlideLocked;

	UPROPERTY()
	bool SlidePulled;

	UPROPERTY()
	bool MagazineEjecting;

	UPROPERTY()
	bool MagazineInserting;

	/** Animation values for our various functions */
	UPROPERTY(replicated, VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float AnimTriggerAlpha;
	UPROPERTY(replicated, VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float AnimSlideAlpha;
	UPROPERTY(replicated, VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float AnimHammerAlpha;
	UPROPERTY(replicated, VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float AnimMagazineAlpha;

	UPROPERTY(Transient)
	float KickbackAlpha;

	/** instant hit notify for replication */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ShotNotify)
	FShotInfo ShotNotify;

	UFUNCTION()
	void OnRep_ShotNotify();

public:
	virtual void Tick(float DeltaSeconds) override;

	struct FHitResult UpdateShotNotify(FVector MuzzleLocation, FVector MuzzleDirection);

	UFUNCTION(reliable, server, WithValidation)
	void ServerFireRound(FVector MuzzleLocation, FVector MuzzleDirection);
	void TriggerPulled();

	void OnSlidePulled();
	void OnSlideReleased();
	UFUNCTION(reliable, server, WithValidation)
	void ServerOnReload();
	void OnReload();

	virtual void GetHandStates(int32& RightHandState, int32& LeftHandState);
	virtual void ItemPickedup(AGunLockCharacter* NewOwner);
	virtual bool RightItemHand() { return true; }
};

