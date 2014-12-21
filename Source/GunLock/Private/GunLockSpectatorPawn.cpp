// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockSpectatorPawn.h"

AGunLockSpectatorPawn::AGunLockSpectatorPawn(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP.SetDefaultSubobjectClass<UGunLockSpectatorPawnMovement>(Super::MovementComponentName))
{
	BaseEyeHeight = 75.5f;
	bCollideWhenPlacing = false;
}


void AGunLockSpectatorPawn::PossessedBy(class AController* NewController)
{
	Controller = NewController;
}