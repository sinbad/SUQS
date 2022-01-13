#include "SuqsWaypointComponent.h"

#include "Suqs.h"
#include "SuqsWaypointSubsystem.h"
#include "Kismet/GameplayStatics.h"


USuqsWaypointComponent::USuqsWaypointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsCurrent = false;
}


// Called when the game starts
void USuqsWaypointComponent::BeginPlay()
{
	Super::BeginPlay();

	// needed for movement events
	bWantsOnUpdateTransform = bRaiseMoveEvents;

	// If quest / task missing, ignore
	if (!QuestID.IsNone() && !TaskID.IsNone() && IsValid(GetWorld()))
	{
		const auto GI = UGameplayStatics::GetGameInstance(this);
		if (IsValid(GI))
		{
			auto Suqs = GI->GetSubsystem<USuqsWaypointSubsystem>();
			Suqs->RegisterWaypoint(this);
		}
	}
	else
	{
		UE_LOG(LogSUQS, Warning, TEXT("Waypoint component %s ignored because it is not initialised"), *GetReadableName());
	}
	
}

void USuqsWaypointComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);

	// Notify 
	if (bIsCurrent && bRaiseMoveEvents)
	{
		OnWaypointMoved.Broadcast(this);
	}
	
}


void USuqsWaypointComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (IsValid(GetWorld()))
	{
		const auto GI = UGameplayStatics::GetGameInstance(this);
		if (IsValid(GI))
		{
			auto Suqs = GI->GetSubsystem<USuqsWaypointSubsystem>();
			Suqs->UnregisterWaypoint(this);
		}
	}
	
}

void USuqsWaypointComponent::SetIsCurrent(bool bNewIsCurrent)
{
	if (bIsCurrent != bNewIsCurrent)
	{
		bIsCurrent = bNewIsCurrent;
		OnWaypointIsCurrentChanged.Broadcast(this);
		
	}
}

void USuqsWaypointComponent::SetEnabled(bool bNewEnabled)
{
	if (bEnabled != bNewEnabled)
	{
		bEnabled = bNewEnabled;
		OnWaypointEnabledChanged.Broadcast(this);
	}
}
