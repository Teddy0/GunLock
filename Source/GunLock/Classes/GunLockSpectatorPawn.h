#pragma once

#include "GameFramework/SpectatorPawn.h"
#include "GunLockSpectatorPawn.generated.h"


UCLASS(config = Game, Blueprintable, BlueprintType)
class AGunLockSpectatorPawn : public ASpectatorPawn
{
	GENERATED_UCLASS_BODY()

	// Begin Pawn overrides
	/** Overridden to avoid changing network role. If subclasses want networked behavior, call the Pawn::PossessedBy() instead. */
	virtual void PossessedBy(class AController* NewController) override;
	// End Pawn overrides
};
