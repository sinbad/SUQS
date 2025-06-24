
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

		FireChangedEvent();

	}

}

void USuqsGameStateComponent::FireChangedEvent()
{
	OnProgressChanged.Broadcast(this, ProgressView);
	if (OnProgressChangedWithDiff.IsBound())
	{
		// Generate diff
		if (USuqsProgressViewHelpers::GetProgressViewDifferences(PreviousProgressView, ProgressView, ProgressDiff))
		{
			OnProgressChangedWithDiff.Broadcast(this, ProgressView, ProgressDiff);
		}	
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
	// We don't actually use the event details, because when replicating we won't have them, so for
	// consistency and simplicity just re-populate the entire thing and generate diffs after
	ProgressView.FromUObject(GetServerProgression());
	FireChangedEvent();
}

void USuqsGameStateComponent::OnRep_Progress()
{
	FireChangedEvent();
}

void USuqsGameStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USuqsGameStateComponent, ProgressView, COND_None, REPNOTIFY_Always);
}
