// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GunLock.h"
#include "GunLock.generated.inl"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, GunLock, "GunLock" );

FLinearColor GetPlayerColor(uint32 SkinId)
{
	if (SkinId == 0)
		return FLinearColor(0.f, 0.f, 0.f);
	else if (SkinId == 1)
		return FLinearColor(0.075f, 0.025f, 0.005f);
	else if (SkinId == 2)
		return FLinearColor(0.5f, 0.f, 0.f);
	else //if (SkinId == 3)
		return FLinearColor(0.03f, 0.05f, 0.23f);
}
