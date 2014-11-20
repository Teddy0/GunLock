// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockWeapon.h"
#include "Net/UnrealNetwork.h"

AGunLockWeapon::AGunLockWeapon(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	GunMesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("GunMesh"));
	GunMesh->AttachParent = RootComponent;
	GunMesh->bGenerateOverlapEvents = false;
	GunMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	AnimHammerAlpha = 1.0f;
	AnimSlideAlpha = 1.0f;
	AnimMagazineAlpha = 1.0f;
	RoundChambered = false;
	SlideLocked = true;
}

void AGunLockWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGunLockWeapon, ShotNotify, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockWeapon, AnimHammerAlpha, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockWeapon, AnimSlideAlpha, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockWeapon, AnimMagazineAlpha, COND_SkipOwner);
}

void AGunLockWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AGunLockCharacter* PlayerOwner = Cast<AGunLockCharacter>(GetOwner());
	bool bIsControlledLocally = PlayerOwner && PlayerOwner->GetController() && PlayerOwner->GetController()->IsLocalPlayerController();

	//Only need to simulate this on the client
	if (bIsControlledLocally)
	{
		KickbackAlpha = FMath::Clamp(KickbackAlpha - DeltaSeconds * 3.5f, 0.0f, 3.0f);
	}

	if (Role == ROLE_Authority || bIsControlledLocally)
	{
		//Animate slide
		if (SlidePulled && PlayerOwner && PlayerOwner->bLeftHandInPosition)
		{
			AnimSlideAlpha = FMath::Clamp(AnimSlideAlpha + DeltaSeconds * 8.0f, 0.0f, 1.0f);

			if (AnimSlideAlpha == 1.0f)
			{
				//If the slide has been opened with a round in the chamber, eject it!
				if (RoundChambered)
				{
					RoundChambered = false;
					//TODO: Spawn a bullet and throw it on the ground!
				}

				if (!AttachedMagazine || AttachedMagazine->Rounds == 0)
				{
					SlideLocked = true;
				}
			}
		}
		else if (AnimSlideAlpha > 0.f && (SlideLocked == false || AnimSlideAlpha < 1.0f))
		{
			AnimSlideAlpha = FMath::Clamp(AnimSlideAlpha - DeltaSeconds * 24.0f, 0.0f, 1.0f);

			//If the slide has returned and we have another round, chamber it
			if (AnimSlideAlpha == 0.f && AttachedMagazine && AttachedMagazine->Rounds > 0)
			{
				AttachedMagazine->Rounds--;
				AttachedMagazine->UpdateBulletMaterials();
				RoundChambered = true;
			}
		}

		//Animate magazine
		if (MagazineEjecting)
		{
			//Only start ejecting the magazine once the left hand has moved it to the starting position
			if (AnimMagazineAlpha != 0.0f || (PlayerOwner->LeftHandPose == LeftHand_InsertMagazine && PlayerOwner->bLeftHandInPosition))
			{
				//Play the magazine out effect
				if (AnimMagazineAlpha == 0.f)
					UGameplayStatics::PlaySound(this, MagazineOutSound, GunMesh, TEXT("Magazine"), false, 1.0f, 1.0f);

				AnimMagazineAlpha = FMath::Clamp(AnimMagazineAlpha + DeltaSeconds * 4.0f, 0.0f, 1.0f);
			}

			if (AnimMagazineAlpha == 1.0f)
			{
				AttachedMagazine->ItemPickedup(PlayerOwner);
				AttachedMagazine = NULL;
				MagazineEjecting = false;
			}
		}
		else if (MagazineInserting)
		{
			//Only start inserting the magazine once the left hand has moved it to the starting position
			if (AnimMagazineAlpha != 1.0f || (PlayerOwner->LeftHandPose == LeftHand_InsertMagazine && PlayerOwner->bLeftHandInPosition))
			{
				//Play the magazine out effect
				if (AnimMagazineAlpha == 1.f)
					UGameplayStatics::PlaySound(this, MagazineInSound, GunMesh, TEXT("Magazine"), false, 1.0f, 1.0f);

				AnimMagazineAlpha = FMath::Clamp(AnimMagazineAlpha - DeltaSeconds * 4.0f, 0.0f, 1.0f);
			}

			if (AnimMagazineAlpha == 0.f)
			{
				AttachedMagazine->AttachToGun(this);
				PlayerOwner->LeftHandItem = NULL;
				MagazineInserting = false;
			}
		}
	}
}

void AGunLockWeapon::GetHandStates(int32& RightHandState, int32& LeftHandState)
{
	if (SlidePulled)
	{
		RightHandState = RightHand_M9_Slide;
		LeftHandState = LeftHand_M9_Slide;
	}
	else if (MagazineEjecting)
	{
		RightHandState = RightHand_M9_Empty;
		LeftHandState = LeftHand_InsertMagazine;
	}
	else if (MagazineInserting)
	{
		RightHandState = RightHand_M9_Empty;
		if (AnimMagazineAlpha >= 0.5f)
			LeftHandState = LeftHand_InsertMagazine;
	}
	else
	{
		RightHandState = RightHand_M9_Ready;
		LeftHandState = LeftHand_M9_Ready;
	}
}

bool AGunLockWeapon::ServerOnReload_Validate()
{
	return true;
}

void AGunLockWeapon::ServerOnReload_Implementation()
{
	OnReload();
}

void AGunLockWeapon::OnReload()
{
	if (Role != ROLE_Authority)
	{
		//Tell the server to run this function
		ServerOnReload();
	}

	AGunLockCharacter* PlayerOwner = Cast<AGunLockCharacter>(GetOwner());
	check(PlayerOwner);

	//Eject the magazine
	if (AttachedMagazine)
	{
		if (!MagazineEjecting && PlayerOwner->LeftHandItem == NULL)
		{
			PlayerOwner->LeftHandItem = AttachedMagazine;
			MagazineEjecting = true;
		}
	}
	else if (PlayerOwner->LeftHandItem)
	{
		if (!MagazineInserting)
		{
			AttachedMagazine = Cast<AGunLockMagazine>(PlayerOwner->LeftHandItem);
			if (AttachedMagazine)
			{
				MagazineInserting = true;
			}
		}
	}
}

void AGunLockWeapon::ItemPickedup(AGunLockCharacter* NewOwner)
{
	if (SpawnPoint)
	{
		AGunLockGameMode* GameMode = Cast<AGunLockGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->SpawnedWeaponCount--;
		}
	}

	Super::ItemPickedup(NewOwner);

	//If this weapon already has a magazine, and they have one in hand, destroy it.
	if (NewOwner->LeftHandItem != NULL && AttachedMagazine != NULL)
	{
		NewOwner->LeftHandItem->Destroy();
		NewOwner->LeftHandItem = NULL;
	}
}

void AGunLockWeapon::TriggerPulled()
{
	if (RoundChambered)
	{
		//BOOM! Fire a bullet
		RoundChambered = false;

		FVector MuzzleLocation = GunMesh->GetSocketLocation(TEXT("Muzzle"));
		FRotator MuzzleRotation = GunMesh->GetSocketRotation(TEXT("Muzzle"));
		FVector MuzzleDirection = MuzzleRotation.Vector();

		//Execute firing on the server
		ServerFireRound(MuzzleLocation, MuzzleDirection);

		//Play the effect on this client (the owner)
		UpdateShotNotify(MuzzleLocation, MuzzleDirection);
		OnRep_ShotNotify();
	}
}

bool AGunLockWeapon::ServerFireRound_Validate(FVector MuzzleLocation, FVector MuzzleDirection)
{
	//TODO: Check the MuzzleLocation is within an acceptable range
	return true;
}
void AGunLockWeapon::ServerFireRound_Implementation(FVector MuzzleLocation, FVector MuzzleDirection)
{
	FHitResult Hit = UpdateShotNotify(MuzzleLocation, MuzzleDirection);

	//Apply damage
	if (Hit.GetActor())
	{
		AGunLockCharacter* PlayerOwner = Cast<AGunLockCharacter>(GetOwner());
		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), 100.f, MuzzleDirection, Hit, PlayerOwner->Controller, this, NULL);
	}

	// play FX locally
	if (GetNetMode() != NM_DedicatedServer)
	{
		OnRep_ShotNotify();
	}
}

FHitResult AGunLockWeapon::UpdateShotNotify(FVector MuzzleLocation, FVector MuzzleDirection)
{
	FHitResult Hit;
	FCollisionQueryParams TraceParams(NAME_None, false, this);
	TraceParams.bReturnPhysicalMaterial = true;
	bool bHit = GetWorld()->LineTraceSingle(Hit, MuzzleLocation, MuzzleLocation + 160000.f * MuzzleDirection, ECC_GameTraceChannel1, TraceParams);
	if (bHit)
	{
		//Play the effect info on other clients (but not the owner)
		ShotNotify.SurfaceType = 0;
		if (Hit.PhysMaterial.IsValid())
			ShotNotify.SurfaceType = (int32)Hit.PhysMaterial->SurfaceType;

		//Play blood impact fx
		if (Hit.GetActor() && Hit.GetActor()->IsA(AGunLockCharacter::StaticClass()))
			ShotNotify.SurfaceType = 5;
	}
	else
	{
		ShotNotify.SurfaceType = -1;
	}
	ShotNotify.Origin = MuzzleLocation;
	ShotNotify.HitLocation = Hit.Location;
	ShotNotify.HitNormal = Hit.Normal;

	//Kick off the animations
	AnimHammerAlpha = 1.0f;
	AnimSlideAlpha = 1.0f;
	KickbackAlpha += 1.0f;

	//If there's no more bullets to feed in, activate the slide lock
	SlideLocked = !(AttachedMagazine && AttachedMagazine->Rounds > 0);

	return Hit;
}

void AGunLockWeapon::OnRep_ShotNotify()
{
	//Create an impact decal/effect!
	if (ShotNotify.SurfaceType > -1)
	{
		USoundCue* ImpactSound = ConcreteImpactSound;
		UParticleSystem* ImpactParticle = ConcreteImpactEffect;
		if (ShotNotify.SurfaceType == SurfaceType2)
		{
			ImpactSound = MetalImpactSound;
			ImpactParticle = MetalImpactEffect;
		}
		else if (ShotNotify.SurfaceType == SurfaceType3)
		{
			ImpactSound = MetalSolidImpactSound;
			ImpactParticle = MetalImpactEffect;
		}
		else if (ShotNotify.SurfaceType == SurfaceType4)
		{
			ImpactSound = WoodImpactSound;
			ImpactParticle = WoodImpactEffect;
		}
		else if (ShotNotify.SurfaceType == SurfaceType5)
		{
			ImpactSound = FleshImpactSound;
			ImpactParticle = FleshImpactEffect;
		}

		//Play the bullet impact sound effect
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ShotNotify.HitLocation, 1.f, 1.f, 0.f, NULL);
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactParticle, ShotNotify.HitLocation, ShotNotify.HitNormal.Rotation());
	}

	//Emit the muzzle particle effect
	UGameplayStatics::SpawnEmitterAtLocation(this, MuzzleEffect, ShotNotify.Origin, (ShotNotify.HitLocation - ShotNotify.Origin).Rotation());

	//Create the empty casing particle effect
	FVector CasingLocation = GunMesh->GetSocketLocation(TEXT("Casings"));
	FRotator CasingRotation = GunMesh->GetSocketRotation(TEXT("Casings"));
	UGameplayStatics::SpawnEmitterAtLocation(this, CasingEffect, CasingLocation, CasingRotation);
	UGameplayStatics::PlaySound(this, CasingSound, GunMesh, TEXT("Casings"), false, 1.0f, 1.0f);

	//Play the gunshot sound effect
	UGameplayStatics::PlaySound(this, GunshotSound, GunMesh, TEXT("Muzzle"), false, 1.0f, 1.0f);
}

void AGunLockWeapon::OnSlidePulled()
{
	SlidePulled = true;
	SlideLocked = false;
}

void AGunLockWeapon::OnSlideReleased()
{
	SlidePulled = false;

	//Play the slide release sound
	if (AnimSlideAlpha == 1.f)
		UGameplayStatics::PlaySound(this, SlideReleaseSound, GunMesh, TEXT("Slide"), false, 1.0f, 1.0f);
}