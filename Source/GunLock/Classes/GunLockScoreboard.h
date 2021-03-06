// Teddy0@gmail.com
#pragma once

#include "GunLockScoreboard.generated.h"

#define MAX_PLAYER_COUNT 4

UCLASS()
class AGunLockScoreboard : public AActor
{
	GENERATED_UCLASS_BODY()

	void PostInitializeComponents() override;
	void Tick(float DeltaSeconds) override;

	/** Gun mesh: Represents the player's Magazine */
	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TSubobjectPtr<class UStaticMeshComponent> ScoreboardMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Team)
	int32 TeamNumber;

	UPROPERTY(VisibleAnywhere, Category = TextRenderActor)
	TSubobjectPtr<class UTextRenderComponent> TextComponents[1 + MAX_PLAYER_COUNT];
};
