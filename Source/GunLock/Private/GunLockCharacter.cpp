// Teddy0@gmail.com
#include "GunLock.h"
#include "Net/UnrealNetwork.h"
#include "GunLockCharacter.h"
#include "IHeadMountedDisplay.h"

//////////////////////////////////////////////////////////////////////////
// AGunLockCharacter

AGunLockCharacter::AGunLockCharacter(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
	, bIsRunning(false)
	, bIsAiming(false)
	, bLeftTrigger(false)
	, bRightTrigger(false)
	, bAimEyeRight(true)
{
	// Set size for collision capsule
	CapsuleComponent->InitCapsuleSize(42.f, 96.0f);

	// Force walking movement mode
	CharacterMovement->MovementMode = EMovementMode::MOVE_Walking;

	Health = 100.f;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachParent = CapsuleComponent;
	FirstPersonCameraComponent->RelativeLocation = FVector(10.f, 0, 75.5f); // Position the camera

	// Setup hand poses
	RightHandPose = RightHand_Idle;
	LeftHandPose = LeftHand_Idle;

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh are set in the
	// derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	LocalViewRotation = FRotator(0.f, 0.f, 0.f);
	LocalHeadLocation = FVector(0.f, 0.f, 160.f);
	RightHandLocation = FVector(38.f, 0.f, 32.f);
	RightHandRotation = FRotator(-90.f, 0.f, 0.f);
	LeftHandLocation = FVector(0.f, 0.f, 0.f);
	LeftHandRotation = FRotator(0.f, 0.f, 0.f);
	AimingAlpha = 0.f;
	CrouchingAlpha = 0.f;
	TimeToNextUpdate = 0.f;

	// Tweaking values
	EyeSocketOffset = FVector(10.f, 0.f, 8.f);
	GunOffset = FVector(32.f, 3.2f, -11.7f);
}

void AGunLockCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SkinMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(1);
}

void AGunLockCharacter::Destroyed()
{
	Super::Destroyed();

	//Destroy inventory
	if (RightHandItem)
	{
		RightHandItem->Destroy();
		RightHandItem = NULL;
	}
	if (LeftHandItem)
	{
		LeftHandItem->Destroy();
		LeftHandItem = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGunLockCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	//InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	
	InputComponent->BindAction("Fire", IE_Pressed, this, &AGunLockCharacter::OnTriggerPull);
	InputComponent->BindAction("Fire", IE_Released, this, &AGunLockCharacter::OnTriggerRelease);

	InputComponent->BindAction("Run", IE_Pressed, this, &AGunLockCharacter::OnStartRun);
	InputComponent->BindAction("Run", IE_Released, this, &AGunLockCharacter::OnStopRun);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AGunLockCharacter::OnCrouchButton);

	InputComponent->BindAction("Aim", IE_Pressed, this, &AGunLockCharacter::OnStartAim);
	InputComponent->BindAction("Aim", IE_Released, this, &AGunLockCharacter::OnStopAim);

	InputComponent->BindAxis("MoveForward", this, &AGunLockCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AGunLockCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AGunLockCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AGunLockCharacter::LookUpAtRate);

	//Weapon inputs
	InputComponent->BindAction("Interact", IE_Pressed, this, &AGunLockCharacter::OnInteractButton);
	InputComponent->BindAction("Reload", IE_Pressed, this, &AGunLockCharacter::OnReloadButton);
	InputComponent->BindAction("Holster", IE_Pressed, this, &AGunLockCharacter::OnHolster);
	InputComponent->BindAction("PullSlide", IE_Pressed, this, &AGunLockCharacter::OnPullSlidePressed);
	InputComponent->BindAction("PullSlide", IE_Released, this, &AGunLockCharacter::OnPullSlideReleased);

	//Menu/HMD inputs
	InputComponent->BindAction("ResetHMD", IE_Pressed, this, &AGunLockCharacter::OnHMDResetButton);
	InputComponent->BindAction("AimEyeLeft", IE_Pressed, this, &AGunLockCharacter::OnAimEyeLeft);
	InputComponent->BindAction("AimEyeRight", IE_Released, this, &AGunLockCharacter::OnAimEyeRight);
	InputComponent->BindAction("Menu", IE_Pressed, this, &AGunLockCharacter::OnMenuButton);
	InputComponent->BindAction("Quit", IE_Pressed, this, &AGunLockCharacter::OnQuitButton);
}

float AGunLockCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	if (Health <= 0.f)
	{
		return 0.f;
	}

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		Health -= ActualDamage;
		if (Health <= 0)
		{
			UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(GetMovementComponent());
			if (MovementComponent)
			{
				MovementComponent->ForceReplicationUpdate();
			}

			bIsDead = true;

			OnDeath();
		}
		else
		{
			//PlayHit(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
		}
	}

	return ActualDamage;
}

void AGunLockCharacter::OnRep_ClientOnDeath()
{
	if (bIsDead)
		OnDeath();
}
void AGunLockCharacter::OnDeath()
{
	bool bIsControlledLocally = Controller && Controller->IsLocalPlayerController();
	if (bIsControlledLocally)
	{
		//Play post process, etc
	}

	if (SkinMaterial)
		SkinMaterial->SetScalarParameterValue(TEXT("HeadVisible"), 1.0f);

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController && Role == ROLE_Authority)
	{
		GetWorldTimerManager().SetTimer(PlayerController, &APlayerController::ServerRestartPlayer, 10.f);
	}

	bReplicateMovement = false;
	bTearOff = true;

	SetLifeSpan(120.f);

	DetachFromControllerPendingDestroy();

	// disable collisions on capsule
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

	if (Mesh)
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		Mesh->SetCollisionProfileName(CollisionProfileName);
	}
	SetActorEnableCollision(true);

	// Ragdoll
	Mesh->SetAllBodiesSimulatePhysics(true);
	Mesh->SetSimulatePhysics(true);
	Mesh->WakeAllRigidBodies();
	Mesh->bBlendPhysics = true;
	UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(GetMovementComponent());
	MovementComponent->StopMovementImmediately();
	MovementComponent->DisableMovement();
	MovementComponent->SetComponentTickEnabled(false);
}

void AGunLockCharacter::Suicide()
{
	TakeDamage(10000.f, FDamageEvent(UDamageType::StaticClass()), Controller, NULL);
}

void AGunLockCharacter::OnStartRun()
{
	bIsRunning = true;
	ServerSetIsRunning(bIsRunning);
}

void AGunLockCharacter::OnStopRun()
{
	bIsRunning = false;
	ServerSetIsRunning(bIsRunning);
}

bool AGunLockCharacter::ServerSetIsRunning_Validate(bool bSetIsRunning)
{
	return true;
}

void AGunLockCharacter::ServerSetIsRunning_Implementation(bool bSetIsRunning)
{
	bIsRunning = bSetIsRunning;
}

void AGunLockCharacter::OnCrouchButton()
{
	UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(GetMovementComponent());
	if (MovementComponent)
	{
		MovementComponent->bWantsToCrouch = !MovementComponent->bWantsToCrouch;
	}
}

void AGunLockCharacter::OnStartAim()
{
	AGunLockWeapon* CurrentWeapon = Cast<AGunLockWeapon>(RightHandItem);
	if (CurrentWeapon != NULL)
	{
		bIsAiming = true;
	}

	bLeftTrigger = true;
}

void AGunLockCharacter::OnStopAim()
{
	bIsAiming = false;
	bLeftTrigger = false;
}

void AGunLockCharacter::OnTriggerPull()
{
	//We can only fire when aiming down sights
	AGunLockWeapon* CurrentWeapon = Cast<AGunLockWeapon>(RightHandItem);
	if (CurrentWeapon)
	{
		CurrentWeapon->TriggerPulled();
	}

	bRightTrigger = true;
}

void AGunLockCharacter::OnTriggerRelease()
{
	bRightTrigger = false;
}

void AGunLockCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
}

void AGunLockCharacter::OnInteractButton()
{
	if (CurrentInteractionTarget)
		ServerInteractWithItem(CurrentInteractionTarget);
}

bool AGunLockCharacter::ServerInteractWithItem_Validate(AGunLockItem* InteractionTarget)
{
	return true;
}

void AGunLockCharacter::ServerInteractWithItem_Implementation(AGunLockItem* InteractionTarget)
{
	if (!InteractionTarget || ((InteractionTarget->RightItemHand() && RightHandItem) || (!InteractionTarget->RightItemHand() && LeftHandItem)))
		return;

	AGunLockCharacter* ItemOwner = Cast<AGunLockCharacter>(InteractionTarget->GetOwner());
	if (ItemOwner && !ItemOwner->bIsDead)
		return;

	if (InteractionTarget->RightItemHand())
		RightHandItem = InteractionTarget;
	else
		LeftHandItem = InteractionTarget;

	//Flag it as being ours
	InteractionTarget->ItemPickedup(this);
}

void AGunLockCharacter::DropItem(bool bRightHandItem)
{
	AGunLockItem* DropItem = bRightHandItem ? RightHandItem : LeftHandItem;
	if (!DropItem || DropItem->bItemDropped)
		return;

	//Play client side efffect
	DropItem->DropItem();

	//Tell the server to drop the item
	ServerDropItem(bRightHandItem);
}

bool AGunLockCharacter::ServerDropItem_Validate(bool bRightHandItem)
{
	return (bRightHandItem && RightHandItem) || (!bRightHandItem && LeftHandItem);
}

void AGunLockCharacter::ServerDropItem_Implementation(bool bRightHandItem)
{
	//TODO: Throw item on the ground, or spawn an effect or something?
	if (bRightHandItem)
	{
		RightHandItem->Destroy();
		RightHandItem = NULL;
	}
	else
	{
		LeftHandItem->Destroy();
		LeftHandItem = NULL;
	}
}

void AGunLockCharacter::OnReloadButton()
{
	AGunLockWeapon* CurrentWeapon = Cast<AGunLockWeapon>(RightHandItem);
	if (CurrentWeapon)
	{
		CurrentWeapon->OnReload();
	}
}

void AGunLockCharacter::OnHolster()
{

}

void AGunLockCharacter::OnPullSlidePressed()
{
	AGunLockWeapon* CurrentWeapon = Cast<AGunLockWeapon>(RightHandItem);
	if (CurrentWeapon && LeftHandItem == NULL && !bIsAiming)
	{
		CurrentWeapon->OnSlidePulled();
	}
}

void AGunLockCharacter::OnPullSlideReleased()
{
	AGunLockWeapon* CurrentWeapon = Cast<AGunLockWeapon>(RightHandItem);
	if (CurrentWeapon)
	{
		CurrentWeapon->OnSlideReleased();
	}
}

void AGunLockCharacter::OnHMDResetButton()
{
	if (GEngine->HMDDevice.IsValid())
	{
		//Get the current rotation of the HMD
		GEngine->HMDDevice->ResetOrientationAndPosition(0);
	}
}

void AGunLockCharacter::OnAimEyeLeft()
{
	bAimEyeRight = false;
}

void AGunLockCharacter::OnAimEyeRight()
{
	bAimEyeRight = true;
}

void AGunLockCharacter::OnMenuButton()
{

}

void AGunLockCharacter::OnQuitButton()
{
	if (!GIsEditor)
	{
		FPlatformMisc::RequestExit(0);
	}
}

void AGunLockCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AGunLockPlayerController* const PC = Cast<AGunLockPlayerController>(Controller);

		// find out which way is forward
		const FRotator Rotation = PC ? PC->BodyRotation : GetControlRotation();
		FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AGunLockCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AGunLockPlayerController* const PC = Cast<AGunLockPlayerController>(Controller);

		// find out which way is right
		const FRotator Rotation = PC ? PC->BodyRotation : GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get right vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AGunLockCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGunLockCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//Accessor functions
FVector AGunLockCharacter::GetHeadLocation() const
{
	FTransform RelativeTM = FTransform(Mesh->RelativeRotation, Mesh->RelativeLocation, Mesh->RelativeScale3D);
	return RelativeTM.InverseTransformPosition(LocalHeadLocation);
}
FRotator AGunLockCharacter::GetHeadRotation() const
{
	return FRotator(LocalViewRotation.Roll, LocalViewRotation.Yaw, -LocalViewRotation.Pitch);
}
FVector AGunLockCharacter::GetRightHandLocation() const
{
	FTransform RelativeTM = FTransform(Mesh->RelativeRotation, Mesh->RelativeLocation, Mesh->RelativeScale3D);
	return RelativeTM.InverseTransformPosition(RightHandLocation);
}
FRotator AGunLockCharacter::GetRightHandRotation() const
{
	static FRotator HandAxis(90.f, 0.f, -90.f);
	static FRotator HandOffset(0.f, 90.f, 0.f);
	return (RightHandRotation.Quaternion()*HandAxis.Quaternion()).Rotator() + HandOffset;
}
FVector AGunLockCharacter::GetLeftHandLocation() const
{
	FTransform RelativeTM = FTransform(Mesh->RelativeRotation, Mesh->RelativeLocation, Mesh->RelativeScale3D);
	return RelativeTM.InverseTransformPosition(LeftHandLocation);
}
FRotator AGunLockCharacter::GetLeftHandRotation() const
{
	static FRotator HandAxis(-90.f, 0.f, 90.f);
	static FRotator HandOffset(0.f, 90.f, 0.f);
	return (LeftHandRotation.Quaternion()*HandAxis.Quaternion()).Rotator() + HandOffset;
}

FRotator AGunLockCharacter::GetViewRotation() const
{
	return Super::GetViewRotation();
}

void AGunLockCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//These values are updated on the client and sent to the server, as they involve player input
	DOREPLIFETIME_CONDITION(AGunLockCharacter, LocalHeadLocation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockCharacter, LocalViewRotation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockCharacter, RightHandLocation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockCharacter, RightHandRotation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockCharacter, LeftHandLocation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockCharacter, LeftHandRotation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockCharacter, RightHandPose, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockCharacter, LeftHandPose, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(AGunLockCharacter, AimingAlpha, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockCharacter, CrouchingAlpha, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGunLockCharacter, bIsRunning, COND_SkipOwner);

	//Only set on the server
	DOREPLIFETIME(AGunLockCharacter, Health);
	DOREPLIFETIME(AGunLockCharacter, bIsDead);
	DOREPLIFETIME(AGunLockCharacter, LeftHandItem);
	DOREPLIFETIME(AGunLockCharacter, RightHandItem);
}

void AGunLockCharacter::FindInteractionTargets(FVector& WorldViewLocation, FRotator& WorldViewRotation)
{
	FVector WorldViewDirection = WorldViewRotation.Vector();

	//Search for any pickup items in range
	static float MaximumPickupDistance = 128.f;
	static float MaximumPickupHeight = 128.f + CrouchingAlpha * 64.f;
	float HighestViewDot = 0.707f;
	CurrentInteractionTarget = NULL;
	for (TObjectIterator<AGunLockItem> It; It; ++It)
	{
		AGunLockItem* TestItem = *It;
		AGunLockCharacter* ItemOwner = Cast<AGunLockCharacter>(TestItem->GetOwner());
		if (ItemOwner && !ItemOwner->bIsDead)
			continue;

		//Don't allow picking up an item if we've got something in that hand
		if (TestItem->RightItemHand() == true && RightHandItem != NULL ||
			TestItem->RightItemHand() == false && LeftHandItem != NULL)
			continue;

		//Don't allow picking up if we've got a loaded gun
		AGunLockWeapon* CurrentWeapon = Cast<AGunLockWeapon>(RightHandItem);
		AGunLockMagazine* MagazinePickup = Cast<AGunLockMagazine>(TestItem);
		if (CurrentWeapon && CurrentWeapon->AttachedMagazine && MagazinePickup)
			continue;

		//Get the vector to the item
		FVector ToItem = TestItem->GetActorLocation() - WorldViewLocation;

		//Get the dot product of the 
		float ItemViewDot = FVector::DotProduct(ToItem.SafeNormal(), WorldViewDirection);
		if (ItemViewDot > HighestViewDot && ToItem.Size2D() < MaximumPickupDistance && FMath::Abs(ToItem.Z) < MaximumPickupHeight)
		{
			CurrentInteractionTarget = TestItem;
			HighestViewDot = ItemViewDot;
		}
	}
}

bool AGunLockCharacter::ServerSetPlayerPoseInfo_Validate(FVector NewHeadLocation, FRotator NewHeadRotation, FVector NewRightHandLocation, FRotator NewRightHandRotation, FVector NewLeftHandLocation, FRotator NewLeftHandRotation, int32 NewRightHandPose, int32 NewLeftHandPose)
{
	return true;
}
void AGunLockCharacter::ServerSetPlayerPoseInfo_Implementation(FVector NewHeadLocation, FRotator NewHeadRotation, FVector NewRightHandLocation, FRotator NewRightHandRotation, FVector NewLeftHandLocation, FRotator NewLeftHandRotation, int32 NewRightHandPose, int32 NewLeftHandPose)
{
	SetPlayerPoseInfo(0.f, NewHeadLocation, NewHeadRotation, NewRightHandLocation, NewRightHandRotation, NewLeftHandLocation, NewLeftHandRotation, NewRightHandPose, NewLeftHandPose);
}
void AGunLockCharacter::SetPlayerPoseInfo(float DeltaSeconds, FVector NewHeadLocation, FRotator NewHeadRotation, FVector NewRightHandLocation, FRotator NewRightHandRotation, FVector NewLeftHandLocation, FRotator NewLeftHandRotation, int32 NewRightHandPose, int32 NewLeftHandPose)
{
	LocalHeadLocation = NewHeadLocation;
	LocalViewRotation = NewHeadRotation;
	LeftHandLocation = NewLeftHandLocation;
	LeftHandRotation = NewLeftHandRotation;
	RightHandLocation = NewRightHandLocation;
	RightHandRotation = NewRightHandRotation;
	RightHandPose = NewRightHandPose;
	LeftHandPose = NewLeftHandPose;

	if (Role < ROLE_Authority)
	{
		TimeToNextUpdate -= DeltaSeconds;
		if (TimeToNextUpdate <= 0.f)
		{
			ServerSetPlayerPoseInfo(NewHeadLocation, NewHeadRotation, NewRightHandLocation, NewRightHandRotation, NewLeftHandLocation, NewLeftHandRotation, NewRightHandPose, NewLeftHandPose);
			TimeToNextUpdate += 1.f / 30.f; // Only send this info 30 times per second to cut down on network bandwidth
		}
	}
}

void AGunLockCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AGunLockWeapon* CurrentWeapon = Cast<AGunLockWeapon>(RightHandItem);

	//Only allow the server to tick, or the local player controller to tick
	bool bIsControlledLocally = Controller && Controller->IsLocalPlayerController();
	if (Role == ROLE_Authority || bIsControlledLocally)
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->Tick(DeltaSeconds);
		}

		//Update the aiming animation alpha
		AimingAlpha = FMath::Clamp(AimingAlpha + 4.f * ((bIsAiming && !bIsRunning) ? DeltaSeconds : -DeltaSeconds), 0.f, 1.f);

		//Update the crouching alpha
		UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(GetMovementComponent());
		CrouchingAlpha = FMath::Clamp(CrouchingAlpha + 2.5f * (MovementComponent->IsCrouching() ? DeltaSeconds : -DeltaSeconds), 0.f, 1.f);

		//Update running speed
		MovementComponent->MaxWalkSpeed = FMath::Clamp(MovementComponent->MaxWalkSpeed + 300.f * (bIsRunning ? DeltaSeconds : -DeltaSeconds), 150.f, 450.f);
	}

	if (bIsControlledLocally)
	{
		if (SkinMaterial)
			SkinMaterial->SetScalarParameterValue(TEXT("HeadVisible"), bIsControlledLocally ? 0.0f : 1.0f);

		//Offset for crouching
		FVector CrouchZOffset = FVector(0.f, 0.f, CrouchingAlpha * -61.5f);
		FirstPersonCameraComponent->SetRelativeLocation(FVector(10.f, 0.f, 75.5f) + CrouchZOffset);

		//Only the local player has correct HMD position, so they do the updating
		FVector NeckIKLocation;
		FVector WorldViewLocation;
		FRotator WorldViewRotation;
		//If this is the local player, get the player's view location and rotation, and transmit it to the other players
		GetController()->GetPlayerViewPoint(WorldViewLocation, WorldViewRotation);

		//Update with HMD offset
		if (GEngine->HMDDevice.IsValid() && GEngine->IsStereoscopic3D() && GEngine->HMDDevice->DoesSupportPositionalTracking())
		{
			FQuat HMDOrientation;
			FVector HMDPosition;
			GEngine->HMDDevice->GetCurrentOrientationAndPosition(HMDOrientation, HMDPosition);

			WorldViewLocation += ActorToWorld().TransformVector(HMDPosition);
			NeckIKLocation = WorldViewLocation - WorldViewRotation.Quaternion().RotateVector(EyeSocketOffset);
		}
		else
		{
			NeckIKLocation = WorldViewLocation - ActorToWorld().TransformVector(EyeSocketOffset);
		}

		//Offset from eyes (camera) to the 

		//Convert into local space for transmitting, animation blueprint, etc.
		FVector NewHeadLocation = ActorToWorld().InverseTransformPosition(NeckIKLocation);
		FRotator NewHeadRotation = WorldViewRotation - ActorToWorld().Rotator();

		//Search for interaction targets within range
		FindInteractionTargets(WorldViewLocation, WorldViewRotation);

		//Update the hand states
		int32 NewRightHandPose;
		int32 NewLeftHandPose;
		{
			NewRightHandPose = RightHand_Idle;
			NewLeftHandPose = LeftHand_Idle;
			if (RightHandItem)
			{
				RightHandItem->GetHandStates(NewRightHandPose, NewLeftHandPose);
			}
			if (LeftHandItem)
			{
				LeftHandItem->GetHandStates(NewRightHandPose, NewLeftHandPose);
			}
			if (CurrentInteractionTarget)
			{
				if (CurrentInteractionTarget->RightItemHand())
					NewRightHandPose = RightHand_Grab;
				else
					NewLeftHandPose = LeftHand_Grab;
			}

			//If our hands are empty and we hold the trigger, do the surrender pose
			if (NewLeftHandPose == LeftHand_Idle && bLeftTrigger)
				NewLeftHandPose = LeftHand_Surrender;
			if (NewRightHandPose == RightHand_Idle && bRightTrigger)
				NewRightHandPose = RightHand_Surrender;
		}

		//Default to idle pose, allow logic/states to override
		FVector RightHandTargetLocation;
		FRotator RightHandTargetRotation;
		float MaxRightHandVelocity = 120.f;
		float MaxLeftHandVelocity = 120.f;
		if (NewRightHandPose == RightHand_Idle)
		{
			static FVector RightHandIdleLocation = FVector(0.f, 28.f, -16.f);
			static FRotator RightHandIdleRotation = FRotator(-90.f, 0.f, 0.f);
			RightHandTargetLocation = RightHandIdleLocation + CrouchZOffset;
			RightHandTargetRotation = RightHandIdleRotation;
		}
		else if (NewRightHandPose == RightHand_Grab)
		{
			check(CurrentInteractionTarget);
			FVector ToObject = CurrentInteractionTarget->GetActorLocation() - WorldViewLocation;
			FVector ObjectDirection = ToObject.SafeNormal();
			RightHandTargetLocation = ActorToWorld().InverseTransformPosition(WorldViewLocation + ObjectDirection * FMath::Min(ToObject.Size() - 16.f, 64.f) );
			RightHandTargetRotation = ObjectDirection.Rotation() + FRotator(45.f, 0.f, -90.f) - ActorToWorld().Rotator();
		}
		else if (NewRightHandPose == RightHand_Surrender)
		{
			static FVector RightHandSurrenderLocation = FVector(36.f, 36.f, 64.f);
			static FRotator RightHandSurrenderRotation = FRotator(90.f, 0.f, -90.f);
			RightHandTargetLocation = RightHandSurrenderLocation + CrouchZOffset;
			RightHandTargetRotation = RightHandSurrenderRotation;
		}
		else if (NewRightHandPose == RightHand_M9_Ready)
		{
			static FVector RightHandReadyLocation = FVector(28.f, 6.f, 32.f);
			static FRotator RightHandReadyRotation = FRotator(-40.f, 0.f, 0.f);
			RightHandTargetLocation = RightHandReadyLocation + CrouchZOffset;
			RightHandTargetRotation = RightHandReadyRotation;
		}
		else if (NewRightHandPose == RightHand_M9_Empty)
		{
			static FVector RightHandEmptyLocation = FVector(28.f, 12.f, 32.f);
			static FRotator RightHandEmptyRotation = FRotator(20.f, -15.f, 60.f);
			RightHandTargetLocation = RightHandEmptyLocation + CrouchZOffset;
			RightHandTargetRotation = RightHandEmptyRotation;
		}
		else if (NewRightHandPose == RightHand_M9_Slide)
		{
			static FVector RightHandSlideLocation = FVector(28.f, 4.f, 42.f);
			static FRotator RightHandSlideRotation = FRotator(0.f, 30.f, 0.f);
			RightHandTargetLocation = RightHandSlideLocation + CrouchZOffset;
			RightHandTargetRotation = RightHandSlideRotation;
		}

		if (bIsAiming && !bIsRunning)
		{
			//Note: The CalculateStereoViewOffset applies HMD position as well, so we need to get the original camera position here
			FRotator CameraViewRotation;
			FVector WorldRightHandLocation;
			GetController()->GetPlayerViewPoint(WorldRightHandLocation, CameraViewRotation);
			//Position the gun 32 cm in front of the player's eye and account for the socket's offset from the bone
			WorldRightHandLocation += WorldViewRotation.Quaternion().RotateVector(GunOffset);

			//If we've got head tracking, offset the gun to be infront of the player's right eye. TODO: Make the eye configurable?
			if (GEngine->IsStereoscopic3D())
				GEngine->StereoRenderingDevice->CalculateStereoViewOffset(bAimEyeRight ? eSSP_RIGHT_EYE : eSSP_LEFT_EYE, WorldViewRotation, GetWorld()->GetWorldSettings()->WorldToMeters, WorldRightHandLocation);

			//Copy the position values into the replicated variables (to be sent to other players)
			RightHandTargetLocation = ActorToWorld().InverseTransformPosition(WorldRightHandLocation);
			RightHandTargetRotation = NewHeadRotation;

			//Add in kickback from shooting (todo; kickback in other states?)
			RightHandTargetLocation += CurrentWeapon->KickbackAlpha * -8.f * RightHandTargetRotation.Vector();
			RightHandTargetRotation += CurrentWeapon->KickbackAlpha * FRotator(30.f, 0.f, 0.f);

			MaxRightHandVelocity = 180.f;
			MaxLeftHandVelocity = 180.f;
		}

		//Animate towards the target location
		RightHandTargetRotation.Normalize();
		FVector NewRightHandLocation;
		FRotator NewRightHandRotation;
		{
			float TargetDistance = (RightHandTargetLocation - RightHandLocation).Size();
			float HandVelocity = MaxRightHandVelocity * DeltaSeconds;
			float LerpAlpha = FMath::Clamp(HandVelocity / TargetDistance, 0.f, 1.f);
			NewRightHandLocation = FMath::Lerp(RightHandLocation, RightHandTargetLocation, LerpAlpha);
			NewRightHandRotation = FMath::Lerp(RightHandRotation, RightHandTargetRotation, LerpAlpha);
			NewRightHandRotation.Normalize();
		}

		//Left hand animations
		FVector LeftHandTargetLocation;
		FRotator LeftHandTargetRotation;
		if (NewLeftHandPose == LeftHand_Idle)
		{
			static FVector LeftHandIdleLocation = FVector(0.f, -28.f, -16.f);
			static FRotator LeftHandIdleRotation = FRotator(-90.f, 0.f, 0.f);
			LeftHandTargetLocation = LeftHandIdleLocation + CrouchZOffset;
			LeftHandTargetRotation = LeftHandIdleRotation;
		}
		else if (NewLeftHandPose == LeftHand_Grab)
		{
			check(CurrentInteractionTarget);
			FVector ToObject = CurrentInteractionTarget->GetActorLocation() - WorldViewLocation;
			FVector ObjectDirection = ToObject.SafeNormal();
			LeftHandTargetLocation = ActorToWorld().InverseTransformPosition(WorldViewLocation + ObjectDirection * FMath::Min(ToObject.Size() - 16.f, 64.f));
			LeftHandTargetRotation = ObjectDirection.Rotation() + FRotator(45.f, 0.f, 90.f) - ActorToWorld().Rotator();
		}
		else if (NewLeftHandPose == LeftHand_Drop)
		{
			static FVector LeftHandMagazineLocation = FVector(28.f, -12.f, -16.f);
			static FRotator LeftHandMagazineRotation = FRotator(0.f, 0.f, 0.f);
			LeftHandTargetLocation = LeftHandMagazineLocation + CrouchZOffset;
			LeftHandTargetRotation = LeftHandMagazineRotation;
		}
		else if (NewLeftHandPose == LeftHand_Surrender)
		{
			static FVector LeftHandSurrenderLocation = FVector(36.f, -36.f, 64.f);
			static FRotator LeftHandSurrenderRotation = FRotator(90.f, 0.f, 90.f);
			LeftHandTargetLocation = LeftHandSurrenderLocation + CrouchZOffset;
			LeftHandTargetRotation = LeftHandSurrenderRotation;
		}
		else if (NewLeftHandPose == LeftHand_M9_Ready)
		{
			static FVector ReadyLocationOffset = FVector(1.f, -7.f, -3.f);
			static FRotator ReadyRotationOffset = FRotator(0.f, 0.f, 0.f);
			LeftHandTargetLocation = RightHandTargetLocation + RightHandTargetRotation.Quaternion().RotateVector(ReadyLocationOffset);
			LeftHandTargetRotation = RightHandTargetRotation + ReadyRotationOffset;
		}
		else if (NewLeftHandPose == LeftHand_M9_Slide)
		{
			static FVector SlideOffsetLocation = FVector(-5.f, -10.f, 0.f);
			static FRotator SlideOffsetRotation = FRotator(0.f, 0.f, 90.f);
			check(CurrentWeapon);
			LeftHandTargetLocation = ActorToWorld().InverseTransformPosition(CurrentWeapon->GunMesh->GetSocketLocation(TEXT("Slide"))) + SlideOffsetLocation;
			LeftHandTargetRotation = RightHandTargetRotation + SlideOffsetRotation;
		}
		else if (NewLeftHandPose == LeftHand_InsertMagazine)
		{
			static FVector MagazineSlideLocation = FVector(-8.f, 10.f, 0.f);
			static FRotator MagazineRotation = FRotator(0.f, 0.f, 45.f);
			check(CurrentWeapon);
			LeftHandTargetLocation = ActorToWorld().InverseTransformPosition(CurrentWeapon->GunMesh->GetSocketLocation(TEXT("Magazine"))) + MagazineSlideLocation;
			LeftHandTargetRotation = MagazineRotation;
		}
		else if (NewLeftHandPose == LeftHand_Magazine)
		{
			static FVector LeftHandMagazineLocation = FVector(28.f, -12.f, 32.f);
			static FRotator LeftHandMagazineRotation = FRotator(0.f, 0.f, 15.f);
			LeftHandTargetLocation = LeftHandMagazineLocation + CrouchZOffset;
			LeftHandTargetRotation = LeftHandMagazineRotation;
		}

		//Animate towards the target location
		FVector NewLeftHandLocation;
		FRotator NewLeftHandRotation;
		LeftHandTargetRotation.Normalize();
		{
			float TargetDistance = (LeftHandTargetLocation - LeftHandLocation).Size();
			float HandVelocity = MaxLeftHandVelocity * DeltaSeconds;
			float LerpAlpha = FMath::Clamp(HandVelocity / TargetDistance, 0.f, 1.f);
			NewLeftHandLocation = FMath::Lerp(LeftHandLocation, LeftHandTargetLocation, LerpAlpha);
			NewLeftHandRotation = FMath::Lerp(LeftHandRotation, LeftHandTargetRotation, LerpAlpha);
			NewLeftHandRotation.Normalize();
		}

		//Set the info locally, and send it to the server for replication
		SetPlayerPoseInfo(DeltaSeconds, NewHeadLocation, NewHeadRotation, NewRightHandLocation, NewRightHandRotation, NewLeftHandLocation, NewLeftHandRotation, NewRightHandPose, NewLeftHandPose);

		//Flag when our hand has reached it's animation position
		bLeftHandInPosition = NewLeftHandLocation.Equals(LeftHandTargetLocation);
		bRightHandInPosition = NewRightHandLocation.Equals(RightHandTargetLocation);

		if (bLeftHandInPosition && LeftHandPose == LeftHand_Drop)
		{
			DropItem(false);
		}
	}
}