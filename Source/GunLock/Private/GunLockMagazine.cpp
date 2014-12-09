// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockMagazine.h"
#include "Net/UnrealNetwork.h"

AGunLockMagazine::AGunLockMagazine(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	MagazineMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MagMesh"));
	MagazineMesh->AttachParent = RootComponent;
	MagazineMesh->bGenerateOverlapEvents = false;
	MagazineMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	MaxRounds = 10;
	Rounds = 0;
}

void AGunLockMagazine::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		//Server sets the number of rounds
		Rounds = 1 + FMath::Square(FMath::FRand()) * 4;
	}

	UpdateBulletMaterials();
}

void AGunLockMagazine::ClientUpdateRounds_Implementation(int32 InRounds)
{
	Rounds = InRounds;
	UpdateBulletMaterials();
}

void AGunLockMagazine::OnRep_RoundsUpdated()
{
	UpdateBulletMaterials();
}

void AGunLockMagazine::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGunLockMagazine, Rounds, COND_SkipOwner);
}

void AGunLockMagazine::GetHandStates(int32& RightHandState, int32& LeftHandState)
{
	if (LeftHandState != LeftHand_InsertMagazine)
	{
		LeftHandState = LeftHand_Magazine;

		//If we've got a gun, hold it in the empty state
		if (RightHandState >= RightHand_M9_Ready && RightHandState <= RightHand_M9_SlideLocked)
			RightHandState = RightHand_M9_Empty;
	}
}

bool AGunLockMagazine::CanPickupItem()
{
	return Super::CanPickupItem();
}

void AGunLockMagazine::ItemPickedup(AGunLockCharacter* NewOwner)
{
	if (SpawnPoint)
	{
		AGunLockGameMode* GameMode = Cast<AGunLockGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->SpawnedMagazineCount--;
		}
	}

	Super::ItemPickedup(NewOwner);
}

void AGunLockMagazine::DropItem()
{
	Super::DropItem();

/*	//Client side code for dropping an item
	if (Rounds == 0 && DropItemEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, DropItemEffect, GetActorLocation(), GetActorRotation());
	}*/
}

void AGunLockMagazine::AttachToGun(AGunLockWeapon* NewWeapon)
{
	SetOwner(NewWeapon);
	AttachRootComponentTo(NewWeapon->GunMesh, TEXT("Magazine"), EAttachLocation::SnapToTarget);
}

void AGunLockMagazine::UpdateBulletMaterials()
{
	if (BulletMaterials.Num() == 0)
	{
		BulletMaterials.Empty(5);
		for (int32 i = 1; i < 6; i++)
		{
			BulletMaterials.Add(MagazineMesh->CreateAndSetMaterialInstanceDynamic(i));
		}
	}

	for (int32 i = 0; i < 5; i++)
	{
		if (BulletMaterials[i])
		{
			BulletMaterials[i]->SetScalarParameterValue(TEXT("Visible"), (i < Rounds) ? 1.0f : 0.0f);
		}
	}
}