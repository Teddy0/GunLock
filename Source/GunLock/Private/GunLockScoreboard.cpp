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
		if (i==0)
			TextComponents[i]->Text = TEXT("GunLock v0.2.1");
		else
			TextComponents[i]->Text = TEXT("");
		TextComponents[i]->TextRenderColor = FColor(0, 0, 0);
		TextComponents[i]->WorldSize = 10.f;
		FString SocketName = FString::Printf(TEXT("textline_%i"), i);
		TextComponents[i]->AttachTo(ScoreboardMesh, *SocketName, EAttachLocation::SnapToTarget);
	}
}

void AGunLockScoreboard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<APlayerState*> SortedPlayerStates;
	for (FConstPawnIterator Iterator = GetWorld()->GetPawnIterator(); Iterator; ++Iterator)
	{
		APawn* Pawn = *Iterator;
		if (Pawn->PlayerState)
			SortedPlayerStates.Add(Pawn->PlayerState);
	}
	SortedPlayerStates.Sort([](APlayerState& L, APlayerState& R) { return R.Score < L.Score; });

	for (int32 i = 0; i < MAX_PLAYER_COUNT; i++)
	{
		if (i < SortedPlayerStates.Num())
		{
			uint32 SkinId = SortedPlayerStates[i]->PlayerId % 4;
			TextComponents[i + 1]->TextRenderColor = GetPlayerColor(SkinId) * 0.05f;
			TextComponents[i + 1]->SetText(FString::Printf(TEXT("%s: %i"), *(SortedPlayerStates[i]->PlayerName.Left(10)), FMath::RoundToInt(SortedPlayerStates[i]->Score)));
		}
		else
		{
			TextComponents[i + 1]->SetText(TEXT(""));
		}
	}
}