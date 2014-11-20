// Teddy0@gmail.com
#include "GunLock.h"
#include "IHeadMountedDisplay.h"

//Player Controller class that moves along a Landscape Spline
AGunLockPlayerController::AGunLockPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	CurrentYawInput = 0.f;
	PrevYawInput = 0.f;
	PendingYawDegrees = 0.f;

	DegreesPerTurn = 30.f;
	DegreesPerSecond = 720.f;
}

void AGunLockPlayerController::SpawnPlayerCameraManager()
{
	Super::SpawnPlayerCameraManager();

	if (PlayerCameraManager)
	{
		PlayerCameraManager->bFollowHmdOrientation = true;
	}
}

void AGunLockPlayerController::AddYawInput(float Val)
{
	if (GEngine->HMDDevice.IsValid() && GEngine->IsStereoscopic3D() /*&& EnableVRComfortMode*/)
	{
		if (Val == 0.f)
			return;

		PrevYawInput = CurrentYawInput;
		CurrentYawInput = Val;

		//UE_LOG(LogGunLock, Log, TEXT("CurrentYawInput: %f PrevYawInput: %f"), CurrentYawInput, PrevYawInput);

		//If we've crossed the threshold
		if (FMath::Abs(CurrentYawInput) >= 0.5f && FMath::Abs(PrevYawInput) < 0.5f)
		{
			if (CurrentYawInput > 0)
				PendingYawDegrees += DegreesPerTurn;
			else
				PendingYawDegrees -= DegreesPerTurn;
		}
	}
	else
	{
		Super::AddYawInput(Val);
	}
}

void AGunLockPlayerController::SetInitialLocationAndRotation(const FVector& NewLocation, const FRotator& NewRotation)
{
	Super::SetInitialLocationAndRotation(NewLocation, NewRotation);

	BodyRotation = NewRotation;
}

/* PlayerTick is only called if the PlayerController has a PlayerInput object.  Therefore, it will not be called on servers for non-locally controlled playercontrollers. */
void AGunLockPlayerController::PlayerTick(float DeltaTime)
{
	if (PendingYawDegrees != 0)
	{
		float DegreesThisFrame = FMath::Min(FMath::Abs(PendingYawDegrees), DegreesPerSecond * DeltaTime);
		if (PendingYawDegrees < 0)
			DegreesThisFrame *= -1.f;
		RotationInput.Yaw += !IsLookInputIgnored() ? DegreesThisFrame : 0.f;

		PendingYawDegrees -= DegreesThisFrame;
	}

	Super::PlayerTick(DeltaTime);
}

/**
* Updates the rotation of player, based on ControlRotation after RotationInput has been applied.
* This may then be modified by the PlayerCamera, and is passed to Pawn->FaceRotation().
*/
void AGunLockPlayerController::UpdateRotation(float DeltaTime)
{
	APawn* const P = GetPawnOrSpectator();

	// Calculate Delta to be applied on ViewRotation
	FRotator DeltaRot(RotationInput);
	if (GEngine->HMDDevice.IsValid() && GEngine->IsStereoscopic3D())
	{
		DeltaRot.Pitch = 0;
	}

	FRotator ViewRotation = GetControlRotation();
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ProcessViewRotation(DeltaTime, ViewRotation, DeltaRot);
	}
	SetControlRotation(ViewRotation);

	//Don't allow turning of the body with HMD when stationary (Tank Mode)
	float PlayerVelocity2D = P ? P->GetVelocity().SizeSquared2D() : 1.f;
	if (GEngine->HMDDevice.IsValid() && GEngine->IsStereoscopic3D() && GEngine->HMDDevice->IsHeadTrackingAllowed() && PlayerVelocity2D >= 1.f)
	{
		//Get the current rotation of the HMD
		FQuat HMDOrientation;
		FVector HMDPosition;
		GEngine->HMDDevice->GetCurrentOrientationAndPosition(HMDOrientation, HMDPosition);

		//Cap the amount the player's body turns due to HMD yaw
		const float HMDTurnArc = DegreesPerTurn;
		const float HMDHalfTurnArc = HMDTurnArc * 0.5f;

		float HMDYaw = HMDOrientation.Rotator().Yaw;
		float HMDAbsYaw = FMath::Abs(HMDYaw);
		if (HMDAbsYaw < HMDTurnArc)
		{
			//Scale the amount of Yaw applied, so when looking sideways we're not applying any turning
			//This lets the player look around but also do small course corrections when looking forwards
			float YawScale = 1.f - FMath::Max(0.f, (HMDAbsYaw - HMDHalfTurnArc)) / HMDHalfTurnArc;
			ViewRotation.Yaw += HMDYaw * YawScale;
		}
	}
	BodyRotation = ViewRotation;
	BodyRotation.Pitch = 0;

	if (P)
	{
		P->FaceRotation(BodyRotation, DeltaTime);
	}
}

void AGunLockPlayerController::Suicide()
{
	if (GetPawn() && GetNetMode() == NM_Standalone)
	{
		AGunLockCharacter* MyPawn = Cast<AGunLockCharacter>(GetPawn());
		if (MyPawn)
		{
			MyPawn->Suicide();
		}
	}
}
