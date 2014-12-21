// Teddy0@gmail.com
#include "GunLock.h"
#include "Net/UnrealNetwork.h"

AGunLockPlayerState::AGunLockPlayerState(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	TeamNumber = 0;
}

void AGunLockPlayerState::Reset()
{
	Super::Reset();

	SetTeamNum(0);
}

void AGunLockPlayerState::ClientInitialize(class AController* InController)
{
	Super::ClientInitialize(InController);
}

void AGunLockPlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;
}

int32 AGunLockPlayerState::GetTeamNum() const
{
	return TeamNumber;
}

void AGunLockPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGunLockPlayerState, TeamNumber);
	DOREPLIFETIME(AGunLockPlayerState, NumKills);
	DOREPLIFETIME(AGunLockPlayerState, NumDeaths);
}