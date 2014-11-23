// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockItem.h"
#include "Net/UnrealNetwork.h"

AGunLockItem::AGunLockItem(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
	, bItemDropped(false)
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
	if (OldOwner == NewOwner)
		OldOwner = NULL;
	check(OldOwner == NULL || OldOwner->bIsDead);

	//Take it from their cold, dead hands
	if (OldOwner && OldOwner->LeftHandItem == this)
		OldOwner->LeftHandItem = NULL;
	else if (OldOwner && OldOwner->RightHandItem == this)
		OldOwner->RightHandItem = NULL;

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

void AGunLockItem::DropItem()
{
	bItemDropped = true;

	//Client side code for dropping an item
	if (DropItemEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, DropItemEffect, GetActorLocation(), GetActorRotation());
	}
}