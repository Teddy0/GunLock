// Teddy0@gmail.com
#pragma once

#include "GunLockServerMessage.generated.h"

UCLASS()
class AGunLockServerMessage : public AActor
{
	GENERATED_UCLASS_BODY()

	virtual void BeginPlay() override;
	void PeekNetworkFailureMessages(class UWorld *InWorld, class UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	UPROPERTY(VisibleAnywhere, Category = TextRenderActor)
	TSubobjectPtr<class UTextRenderComponent> TextComponent;
};
