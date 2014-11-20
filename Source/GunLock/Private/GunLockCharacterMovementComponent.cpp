// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockCharacterMovementComponent.h"

UGunLockCharacterMovementComponent::UGunLockCharacterMovementComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	MaxWalkSpeed = 150.0f;
}
