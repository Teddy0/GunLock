// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

enum ERightHandState
{
	RightHand_Idle,
	RightHand_Grab,
	RightHand_Drop,
	RightHand_Surrender,
	RightHand_M9_Ready,
	RightHand_M9_Empty,
	RightHand_M9_Slide,
	RightHand_M9_SlideLocked,
	RightHand_PickupAmmo,
	RightHand_LoadAmmo,
};

enum ELeftHandState
{
	LeftHand_Idle,
	LeftHand_Grab,
	LeftHand_Drop,
	LeftHand_Surrender,
	LeftHand_M9_Ready,
	LeftHand_M9_Slide,
	LeftHand_InsertMagazine,
	LeftHand_Magazine,
	LeftHand_M9_SlideLocked,
	LeftHand_PickupAmmo,
	LeftHand_LoadAmmo
};

#include "Engine.h"
#include "GunLockClasses.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGunLock, Log, All);

FLinearColor GetPlayerColor(uint32 SkinId);