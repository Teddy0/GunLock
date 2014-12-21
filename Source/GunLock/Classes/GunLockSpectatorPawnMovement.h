#pragma once

#include "GunLockSpectatorPawnMovement.generated.h"

UCLASS()
class UGunLockSpectatorPawnMovement : public USpectatorPawnMovement
{
	GENERATED_UCLASS_BODY()

protected:
	/** Update Velocity based on input. Also applies gravity. */
	virtual void ApplyControlInputToVelocity(float DeltaTime);
};

