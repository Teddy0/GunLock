// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GunLockCharacter.generated.h"

UCLASS(config=Game)
class AGunLockCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
	class UMaterialInstanceDynamic* BodyMaterial;

	UPROPERTY()
	class UMaterialInstanceDynamic* SkinMaterial;

	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;

	// Current health of the Pawn
	UPROPERTY(BlueprintReadWrite, Replicated, Category = Health)
	float Health;

	/** Take damage, handle death */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay, ReplicatedUsing = OnRep_ClientOnDeath)
	bool bIsDead;
	UFUNCTION()
	void OnRep_ClientOnDeath();
	UFUNCTION()
	void OnDeath();
	UFUNCTION()
	void Suicide();

	/** Flag to indicate player is holding down the run button */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay, Replicated)
	bool bIsRunning;
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetIsRunning(bool bSetIsRunning);

	UPROPERTY()
	bool bHasSpawnedGunForPlayer;

	/** Which pose the right hand is currently in */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Animation)
	int32 RightHandPose;

	/** Which pose the right hand is currently in */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Animation)
	int32 LeftHandPose;

	UPROPERTY(Replicated)
	class AGunLockItem* LeftHandItem;

	UPROPERTY(Replicated)
	class AGunLockItem* RightHandItem;

	UPROPERTY(Replicated)
	class AGunLockItem* HolsteredItem;

	UPROPERTY()
	bool bReachingForHolster;

	/** The current item within interaction range */
	UPROPERTY()
	class AGunLockItem* CurrentInteractionTarget;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	TSubobjectPtr<class UCameraComponent> FirstPersonCameraComponent;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Interpolate between aiming pose/animatinos */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Animation)
	float AimingAlpha;

	/** When we're running, we start panting which effects aiming */
	UPROPERTY()
	float PantingAlpha;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Animation)
	float CrouchingAlpha;

	/** Helper functions of animation IK, etc*/
	UFUNCTION(BlueprintCallable, Category = Animation)
	FVector GetHeadLocation() const;

	UFUNCTION(BlueprintCallable, Category = Animation)
	FRotator GetHeadRotation() const;

	UFUNCTION(BlueprintCallable, Category = Animation)
	FVector GetRightHandLocation() const;

	UFUNCTION(BlueprintCallable, Category = Animation)
	FRotator GetRightHandRotation() const;

	UFUNCTION(BlueprintCallable, Category = Animation)
	FVector GetLeftHandLocation() const;

	UFUNCTION(BlueprintCallable, Category = Animation)
	FRotator GetLeftHandRotation() const;

	UPROPERTY()
	bool bLeftHandInPosition;

	UPROPERTY()
	bool bRightHandInPosition;

	//These values are here for tweaking
	UPROPERTY()
	FVector EyeSocketOffset;
	UPROPERTY()
	FVector GunOffset;

protected:
	/** Position of player's head in local space */
	UPROPERTY(Replicated)
	FVector LocalHeadLocation;

	/** Rotation of player's eyes in local space */
	UPROPERTY(Replicated)
	FRotator LocalViewRotation;

	/** Position of player's right hand in local space */
	UPROPERTY(Replicated)
	FVector RightHandLocation;

	/** Rotation of player's right hand local space */
	UPROPERTY(Replicated)
	FRotator RightHandRotation;

	/** Position of player's left hand in local space */
	UPROPERTY(Replicated)
	FVector LeftHandLocation;

	/** Rotation of player's left hand local space */
	UPROPERTY(Replicated)
	FRotator LeftHandRotation;

	/** Helper functions for passing variables to the server*/
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetPlayerPoseInfo(FVector NewHeadLocation, FRotator NewHeadRotation, FVector NewRightHandLocation, FRotator NewRightHandRotation, FVector NewLeftHandLocation, FRotator NewLeftHandRotation, int32 NewRightHandPose, int32 NewLeftHandPose);
	void SetPlayerPoseInfo(float DeltaSeconds, FVector NewHeadLocation, FRotator NewHeadRotation, FVector NewRightHandLocation, FRotator NewRightHandRotation, FVector NewLeftHandLocation, FRotator NewLeftHandRotation, int32 NewRightHandPose, int32 NewLeftHandPose);

	UPROPERTY()
	float TimeToNextUpdate;

	/** Attempt to interact with an item*/
	UFUNCTION(reliable, server, WithValidation)
	void ServerInteractWithItem(class AGunLockItem* InteractionTarget);

	UFUNCTION(reliable, server, WithValidation)
	void ServerDropItem(bool bRightHandItem);
	void DropItem(bool bRightHandItem);

	UFUNCTION(reliable, server, WithValidation)
	void ServerHolsterItem(bool Unholstering);
	void HolsterItem();

	/** Flag to indicate player is holding down the aim button */
	UPROPERTY()
	bool bIsAiming;

	UPROPERTY()
	bool bLeftTrigger;

	UPROPERTY()
	bool bRightTrigger;

	/** Flag indicates which eye to aim with */
	UPROPERTY(globalconfig)
	bool bAimEyeRight;

	/** Track reload/holster button presses */
	UPROPERTY()
	float PressedReloadTime;
	UPROPERTY()
	float PressedHolsterTime;

	/** Handler for a touch input beginning. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Fires a projectile. */
	void OnTriggerPull();
	void OnTriggerRelease();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	//sets the run flag when button is pressed
	void OnStartRun();
	
	//clears the run flag when key is released
	void OnStopRun();

	//togggle crouching mode
	void OnCrouchButton();

	//sets the run flag when button is pressed
	void OnStartAim();

	//clears the run flag when key is released
	void OnStopAim();

	void OnInteractButton();
	void OnReloadPressed();
	void OnReloadReleased();
	void OnHolsterPressed();
	void OnHolsterReleased();
	void OnPullSlidePressed();
	void OnPullSlideReleased();

	void OnHMDResetButton();
	void OnAimEyeLeft();
	void OnAimEyeRight();
	void OnMenuButton();
	void OnQuitButton();

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	virtual FRotator GetViewRotation() const override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	void FindInteractionTargets(FVector& WorldViewLocation, FRotator& WorldViewRotation);
public:
	virtual void Tick(float DeltaSeconds) override;
};

