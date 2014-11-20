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

	MaxRounds = 15;
	Rounds = 0;
}

void AGunLockMagazine::BeginPlay()
{
	Super::BeginPlay();

	Rounds = FMath::RandRange(4, MaxRounds);

	BulletMaterials.Empty(5);
	for (int32 i = 1; i < 6; i++)
	{
		BulletMaterials.Add(MagazineMesh->CreateAndSetMaterialInstanceDynamic(i));
	}
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
		if (Rounds == 0)
			LeftHandState = LeftHand_Drop;
		else
			LeftHandState = LeftHand_Magazine;

		//If we've got a gun, hold it in the empty state
		if (RightHandState != RightHand_Idle)
			RightHandState = RightHand_M9_Empty;
	}
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

void AGunLockMagazine::AttachToGun(AGunLockWeapon* NewWeapon)
{
	AttachRootComponentTo(NewWeapon->GunMesh, TEXT("Magazine"), EAttachLocation::SnapToTarget);
}

void AGunLockMagazine::UpdateBulletMaterials()
{
	for (int32 i = 0; i < 5; i++)
	{
		if (BulletMaterials[i])
		{
			BulletMaterials[i]->SetScalarParameterValue(TEXT("Visible"), (i < Rounds) ? 1.0f : 0.0f);
		}
	}
}