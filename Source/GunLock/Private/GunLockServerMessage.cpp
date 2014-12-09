// Teddy0@gmail.com
#include "GunLock.h"
#include "GunLockServerMessage.h"

AGunLockServerMessage::AGunLockServerMessage(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	TextComponent = PCIP.CreateDefaultSubobject<UTextRenderComponent>(this, TEXT("TextRenderComponent"));
	TextComponent->Text = TEXT("Connecting...");
	RootComponent = TextComponent;
}

void AGunLockServerMessage::BeginPlay()
{
	Super::BeginPlay();

	//GEngine->OnTravelFailure().AddUObject(this, &UGameViewportClient::PeekTravelFailureMessages);
	GEngine->OnNetworkFailure().AddUObject(this, &AGunLockServerMessage::PeekNetworkFailureMessages);
}

void AGunLockServerMessage::PeekNetworkFailureMessages(UWorld *InWorld, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	TextComponent->SetText(FString::Printf(TEXT("Connection failed: [%s] %s"), ENetworkFailure::ToString(FailureType), *ErrorString) );
}
