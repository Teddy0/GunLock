// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockItem.h"
#include "Net/UnrealNetwork.h"

AGunLockItem::AGunLockItem(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	TSubobjectPtr<USceneComponent> SceneComponent = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComp"));
	RootComponent = SceneComponent;

	bNetUseOwnerRelevancy = true;
	bReplicates = true;
	bReplicateMovement = true;
}

void AGunLockItem::ItemPickedup(AGunLockCharacter* NewOwner)
{
	AGunLockCharacter* OldOwner = Cast<AGunLockCharacter>(GetOwner());
	if (OldOwner && OldOwner != NewOwner)
	{
		//Take it from their cold, dead hands
		check(OldOwner->bIsDead);
		if (OldOwner->LeftHandItem == this)
			OldOwner->LeftHandItem = NULL;
		else if (OldOwner->RightHandItem == this)
			OldOwner->RightHandItem = NULL;
		else if (OldOwner->HolsteredItem == this)
			OldOwner->HolsteredItem = NULL;
	}

	SetOwner(NewOwner);

	if (RightItemHand())
		AttachRootComponentTo(NewOwner->Mesh, TEXT("RightHandItem"), EAttachLocation::SnapToTarget);
	else
		AttachRootComponentTo(NewOwner->Mesh, TEXT("LeftHandItem"), EAttachLocation::SnapToTarget);

	if (SpawnPoint)
	{
		AGunLockGameMode* GameMode = Cast<AGunLockGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->ItemSpawnPoints.Remove(SpawnPoint);
			GameMode->EmptyItemSpawnPoints.Add(SpawnPoint);
		}
		SpawnPoint = NULL;
	}
}

void AGunLockItem::NotifyOwnerDied()
{
}

bool AGunLockItem::CanPickupItem()
{
	AActor* Owner = GetOwner();
	AGunLockCharacter* PlayerOwner = Cast<AGunLockCharacter>(Owner);
	return Owner == NULL || (PlayerOwner && PlayerOwner->bIsDead);
}

void AGunLockItem::DropItem()
{
}

void AGunLockItem::NetMulticast_PlaySound_Implementation(USoundCue* Sound, bool bFollow)
{
	UGameplayStatics::PlaySound(this, Sound, RootComponent, NAME_None, bFollow, 1.0f, 1.0f);
}