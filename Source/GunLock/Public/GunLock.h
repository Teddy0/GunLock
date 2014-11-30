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
};

#include "Engine.h"
#include "GunLockClasses.h"

FLinearColor GetPlayerColor(uint32 SkinId);