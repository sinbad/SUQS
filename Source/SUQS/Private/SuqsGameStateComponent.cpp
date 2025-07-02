
#include "SuqsGameStateComponent.h"
#include "SuqsProgression.h"
#include "Net/UnrealNetwork.h"

USuqsGameStateComponent::USuqsGameStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	
}

void USuqsGameStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only the server needs to tick, as it collates updates from real progress
	SetComponentTickEnabled(GetOwner()->HasAuthority());
	
}

void USuqsGameStateComponent::InitServerProgress()
{
	if (!ServerProgression && GetOwner()->HasAuthority())
	{
		ServerProgression = NewObject<USuqsProgression>(this, "ServerProgression");
		ProgressView.FromUObject(ServerProgression, bIncludeCompletedObjectives);
		ServerProgression->OnProgressionEvent.AddDynamic(this, &USuqsGameStateComponent::OnProgressionEvent);
		bServerPendingChanges = false;

		FireChangedEvent();

	}
}

void USuqsGameStateComponent::TickComponent(float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwner()->HasAuthority() && bServerPendingChanges)
	{
		ProgressView.FromUObject(GetServerProgression(), bIncludeCompletedObjectives);
		FireChangedEvent();
		bServerPendingChanges = false;
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
		PreviousProgressView = ProgressView;
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
	// To merge multiple change events in a tick we just mark this as dirty
	bServerPendingChanges = true;
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
