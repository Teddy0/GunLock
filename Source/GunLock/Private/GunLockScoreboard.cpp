// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockScoreboard.h"
#include "Net/UnrealNetwork.h"

AGunLockScoreboard::AGunLockScoreboard(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
	ScoreboardMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("ScoreboardMesh"));
	RootComponent = ScoreboardMesh;

	for (int32 i = 0; i < 1 + MAX_PLAYER_COUNT; i++)
	{
		TextComponents[i] = PCIP.CreateDefaultSubobject<UTextRenderComponent>(this, *FString::Printf(TEXT("TextRenderComponent_%i"), i));
		TextComponents[i]->Text = TEXT("");
		TextComponents[i]->TextRenderColor = FColor(0,0,0);
		TextComponents[i]->WorldSize = 10.f;
		FString SocketName = FString::Printf(TEXT("textline_%i"), i);
		TextComponents[i]->AttachTo(ScoreboardMesh, *SocketName, EAttachLocation::SnapToTarget);
	}
}

void AGunLockScoreboard::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (TeamNumber == 0)
	{
		TextComponents[0]->SetText(TEXT("GunLock v0.2.2"));
	}
	else if (TeamNumber == 1)
	{
		TextComponents[0]->SetText(TEXT("Blue Team"));
	}
	else if (TeamNumber == 2)
	{
		TextComponents[0]->SetText(TEXT("Red Team"));
	}

	TextComponents[0]->TextRenderColor = GetPlayerColor(TeamNumber) * 0.05f;
}

void AGunLockScoreboard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<AGunLockPlayerState*> SortedPlayerStates;
	for (FConstPawnIterator Iterator = GetWorld()->GetPawnIterator(); Iterator; ++Iterator)
	{
		APawn* Pawn = *Iterator;
		AGunLockPlayerState* PlayerState = Cast<AGunLockPlayerState>(Pawn->PlayerState);
		if (PlayerState && PlayerState->GetTeamNum() == TeamNumber)
			SortedPlayerStates.Add(PlayerState);
	}
	SortedPlayerStates.Sort([](APlayerState& L, APlayerState& R) { return R.Score < L.Score; });

	for (int32 i = 0; i < MAX_PLAYER_COUNT; i++)
	{
		if (i < SortedPlayerStates.Num())
		{
			TextComponents[i + 1]->TextRenderColor = GetPlayerColor(TeamNumber) * 0.05f;
			TextComponents[i + 1]->SetText(FString::Printf(TEXT("%s: %i"), *(SortedPlayerStates[i]->PlayerName.Left(10)), FMath::RoundToInt(SortedPlayerStates[i]->Score)));
		}
		else
		{
			TextComponents[i + 1]->SetText(TEXT(""));
		}
	}
}