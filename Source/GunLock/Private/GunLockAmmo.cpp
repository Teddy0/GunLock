// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockAmmo.h"
#include "Net/UnrealNetwork.h"

AGunLockAmmo::AGunLockAmmo(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	AmmoMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("AmmoMesh"));
	AmmoMesh->AttachParent = RootComponent;

	MaxRounds = 8;
	Rounds = 0;
}

void AGunLockAmmo::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		//Server sets the number of rounds
		Rounds = 1 + FMath::Square(FMath::FRand()) * (MaxRounds - 1);
	}

	UpdateBulletMaterials();
}

void AGunLockAmmo::OnRep_RoundsUpdated()
{
	UpdateBulletMaterials();
}

void AGunLockAmmo::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGunLockAmmo, Rounds);
}

void AGunLockAmmo::GetHandStates(int32& RightHandState, int32& LeftHandState)
{
	if (bPickupInRightHand)
		RightHandState = RightHand_PickupAmmo;
	else
		LeftHandState = LeftHand_PickupAmmo;
}

void AGunLockAmmo::ItemPickedup(AGunLockCharacter* NewOwner)
{
	if (SpawnPoint)
	{
		AGunLockGameMode* GameMode = Cast<AGunLockGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->SpawnedAmmoCount--;
		}
	}

	Super::ItemPickedup(NewOwner);
}

void AGunLockAmmo::AmmoPickedup(AGunLockCharacter* NewOwner)
{
	//Play a sound for ammo pickup
	//NetMulticast_PlaySound_Implementation();

	NewOwner->Ammo += Rounds;
	Destroy();
}

void AGunLockAmmo::UpdateBulletMaterials()
{
	if (BulletMaterials.Num() == 0)
	{
		BulletMaterials.Empty(MaxRounds);
		for (int32 i = 0; i < MaxRounds; i++)
		{
			BulletMaterials.Add(AmmoMesh->CreateAndSetMaterialInstanceDynamic(i));
		}
	}

	for (int32 i = 0; i < MaxRounds; i++)
	{
		if (BulletMaterials[i])
		{
			BulletMaterials[i]->SetScalarParameterValue(TEXT("Visible"), (i < Rounds) ? 1.0f : 0.0f);
		}
	}
}