// Teddy0@gmail.com
#pragma once

#include "GunLockLocalPlayer.generated.h"

UCLASS(config = Engine)
class UGunLockLocalPlayer : public ULocalPlayer
{
	GENERATED_UCLASS_BODY()
public:

	virtual FString GetNickname() const override;

	UPROPERTY(Config)
	FString PlayerName;
};



