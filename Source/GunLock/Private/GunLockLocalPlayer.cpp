// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockLocalPlayer.h"

UGunLockLocalPlayer::UGunLockLocalPlayer(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
}

FString UGunLockLocalPlayer::GetNickname() const
{
	return PlayerName;
}