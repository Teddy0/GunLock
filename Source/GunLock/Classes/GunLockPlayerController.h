// Teddy0@gmail.com

#pragma once

#include "GunLockPlayerController.generated.h"

/**
*
*/
UCLASS(config = Game)
class AGunLockPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

	/** Degrees to turn for each joystick press */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerController)
	float DegreesPerTurn;

	/** Speed to turn the camera in degrees per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerController)
	float DegreesPerSecond;

	//Transient variables
	UPROPERTY()
	FRotator BodyRotation;

	UPROPERTY()
	float PendingYawDegrees;

	UPROPERTY()
	float PrevYawInput;

	UPROPERTY()
	float CurrentYawInput;

	UPROPERTY(globalconfig)
	bool VRComfortMode;

public:
	virtual void SpawnPlayerCameraManager() override;

	/** Add Yaw (turn) input */
	virtual void AddYawInput(float Val) override;

	/**
	* Processes player input (immediately after PlayerInput gets ticked) and calls UpdateRotation().
	* PlayerTick is only called if the PlayerController has a PlayerInput object. Therefore, it will only be called for locally controlled PlayerControllers.
	*/
	virtual void PlayerTick(float DeltaTime);

	//Init functions
	virtual void SetInitialLocationAndRotation(const FVector& NewLocation, const FRotator& NewRotation);

	/**
	* Updates the rotation of player, based on ControlRotation after RotationInput has been applied.
	* This may then be modified by the PlayerCamera, and is passed to Pawn->FaceRotation().
	*/
	virtual void UpdateRotation(float DeltaTime) override;

	//For debugging
	UFUNCTION(exec)
	virtual void Suicide();
};