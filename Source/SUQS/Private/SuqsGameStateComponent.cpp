
#include "SuqsGameStateComponent.h"
#include "SuqsProgression.h"
#include "Net/UnrealNetwork.h"

USuqsGameStateComponent::USuqsGameStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
}

void USuqsGameStateComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void USuqsGameStateComponent::InitServerProgress()
{
	if (!ServerProgression && GetOwner()->HasAuthority())
	{
		ServerProgression = NewObject<USuqsProgression>(this, "ServerProgression");
		ProgressView.FromUObject(ServerProgression);
		ServerProgression->OnProgressionEvent.AddDynamic(this, &USuqsGameStateComponent::OnProgressionEvent);
		OnProgressChanged.Broadcast(this, ProgressView);
	}

}

USuqsProgression* USuqsGameStateComponent::GetServerProgression()
{
	checkf (GetOwner()->HasAuthority(), TEXT("You cannot call GetServerProgression from a client"))

	InitServerProgress();
	
	return ServerProgression;
}

void USuqsGameStateComponent::OnProgressionEvent(const FSuqsProgressionEventDetails& Details)
{
	ProgressView.FromUObject(GetServerProgression());
	OnProgressChanged.Broadcast(this, ProgressView);
}

void USuqsGameStateComponent::OnRep_Progress()
{
	OnProgressChanged.Broadcast(this, ProgressView);
}

void USuqsGameStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USuqsGameStateComponent, ProgressView, COND_None, REPNOTIFY_Always);
}
