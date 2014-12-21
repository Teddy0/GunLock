// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockSpectatorPawnMovement.h"

UGunLockSpectatorPawnMovement::UGunLockSpectatorPawnMovement(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	MaxSpeed = 400.f;
}

void UGunLockSpectatorPawnMovement::ApplyControlInputToVelocity(float DeltaTime)
{
	Super::ApplyControlInputToVelocity(DeltaTime);
	Velocity.Z = 0.f;
}