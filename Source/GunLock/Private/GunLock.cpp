// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GunLock.h"
#include "GunLock.generated.inl"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, GunLock, "GunLock" );
DEFINE_LOG_CATEGORY(LogGunLock);

FLinearColor GetPlayerColor(uint32 SkinId)
{
	if (SkinId == 1)
		return FLinearColor(0.03f, 0.05f, 0.23f); 
	else if (SkinId == 2)
		return FLinearColor(0.5f, 0.f, 0.f);

	return FLinearColor(0.f, 0.f, 0.f);
}
