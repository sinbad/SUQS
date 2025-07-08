#include "SuqsWaypointComponent.h"

#include "Suqs.h"
#include "SuqsWaypointSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


USuqsWaypointComponent::USuqsWaypointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsCurrent = false;

	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void USuqsWaypointComponent::BeginPlay()
{
	Super::BeginPlay();

	// needed for movement events
	bWantsOnUpdateTransform = bRaiseMoveEvents;

	Register();
	
}

void USuqsWaypointComponent::Register()
{
	// Only link with quest on the server & if set up
	if (GetOwnerRole() == ROLE_Authority && !bIsRegistered)
	{
		// If quest / task missing, ignore
		if (!QuestID.IsNone() && !TaskID.IsNone() && IsValid(GetWorld()))
		{
			const auto GI = UGameplayStatics::GetGameInstance(this);
			if (IsValid(GI))
			{
				auto Suqs = GI->GetSubsystem<USuqsWaypointSubsystem>();
				Suqs->RegisterWaypoint(this);
				bIsRegistered = true;
			}
		}
	}
}

void USuqsWaypointComponent::Unregister()
{
	if (GetOwnerRole() == ROLE_Authority && bIsRegistered)
	{
		if (IsValid(GetWorld()))
		{
			const auto GI = UGameplayStatics::GetGameInstance(this);
			if (IsValid(GI))
			{
				auto Suqs = GI->GetSubsystem<USuqsWaypointSubsystem>();
				Suqs->UnregisterWaypoint(this);
				bIsRegistered = false;
			}
		}
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

	Unregister();
	
}



void USuqsWaypointComponent::SetIsCurrent(bool bNewIsCurrent)
{
	if (bIsCurrent != bNewIsCurrent)
	{
		bIsCurrent = bNewIsCurrent;
		OnIsCurrentChanged();
		
	}
}

void USuqsWaypointComponent::OnIsCurrentChanged()
{
	OnWaypointIsCurrentChanged.Broadcast(this);
}

void USuqsWaypointComponent::SetEnabled(bool bNewEnabled)
{
	if (bEnabled != bNewEnabled)
	{
		bEnabled = bNewEnabled;
		OnIsEnabledChanged();
	}
}

void USuqsWaypointComponent::OnIsEnabledChanged()
{
	OnWaypointEnabledChanged.Broadcast(this);
}

void USuqsWaypointComponent::Initialise(FName InQuestID, FName InTaskID, uint8 InSequenceIndex)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (!InQuestID.IsNone() && !InTaskID.IsNone())
		{
			if (bIsRegistered)
			{
				Unregister();
			}
			QuestID = InQuestID;
			TaskID = InTaskID;
			SequenceIndex = InSequenceIndex;
			Register();
		}
	}
	else
	{
		UE_LOG(LogSUQS, Warning, TEXT("Called Waypoint component Initialise() from non-Server, ignoring"));
	}
}

void USuqsWaypointComponent::OnRep_Enabled()
{
	// Multiplayer notify
	OnIsEnabledChanged();
}

void USuqsWaypointComponent::OnRep_IsCurrent()
{
	// Multiplayer notify
	OnIsCurrentChanged();
}

void USuqsWaypointComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USuqsWaypointComponent, bEnabled, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USuqsWaypointComponent, bIsCurrent, COND_None, REPNOTIFY_Always);
}
