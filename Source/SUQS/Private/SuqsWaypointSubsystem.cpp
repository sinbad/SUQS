#include "SuqsWaypointSubsystem.h"
#include "SuqsProgression.h"
#include "SuqsWaypointComponent.h"

void USuqsWaypointSubsystem::RegisterWaypoint(USuqsWaypointComponent* Waypoint)
{
	auto& List = WaypointsByQuest.FindOrAdd(Waypoint->GetQuestID());

	// group by task, then if there's >1 waypoint for a single task, order by sequence index for later convenience
	bool bInserted = false;
	bool bExistingTask = false;
	for (int i = 0; i < List.Num(); ++i)
	{
		const auto Curr = List[i];

		// Avoid duplicates
		if (Curr == Waypoint)
			bInserted = true;
		
		if (Curr->GetTaskID() == Waypoint->GetTaskID())
		{
			bExistingTask = true;
		}

		// If we've seen a waypoint for this task previously:
		// Insert here if we're now on a diff task (thus, group)
		//  or if we're on the same task and our sequence index is before this one
		if (bExistingTask &&
			(Curr->GetSequenceIndex() > Waypoint->GetSequenceIndex() ||
			Curr->GetTaskID() != Waypoint->GetTaskID()))
		{
			List.Insert(Waypoint, i);
			bInserted = true;
			break;
		}

	}

	if (!bInserted)
	{
		List.Add(Waypoint);
		bInserted = true;
	}

	if (bInserted)
	{
		// Initialise the waypoint IsCurrent state from quest progression
		// This isn't saved in the waypoint savegame data because it's derived from quest progression and
		// may be out of date from when it was last saved (making progress while the level is unloaded)
		if (Progression.IsValid())
		{
			Waypoint->SetIsCurrent(Progression->IsTaskRelevant(Waypoint->GetQuestID(), Waypoint->GetTaskID()));
		}
		Waypoint->OnWaypointMoved.AddDynamic(this, &USuqsWaypointSubsystem::OnWaypointMoved);
		Waypoint->OnWaypointEnabledChanged.AddDynamic(this, &USuqsWaypointSubsystem::OnWaypointEnabledChanged);
		Waypoint->OnWaypointIsCurrentChanged.AddDynamic(this, &USuqsWaypointSubsystem::OnWaypointIsCurrentChanged);
	}
}

void USuqsWaypointSubsystem::UnregisterWaypoint(USuqsWaypointComponent* Waypoint)
{
	const auto pList = WaypointsByQuest.Find(Waypoint->GetQuestID());
	if (pList)
	{
		pList->RemoveSingle(Waypoint);
		Waypoint->OnWaypointMoved.RemoveDynamic(this, &USuqsWaypointSubsystem::OnWaypointMoved);
		Waypoint->OnWaypointEnabledChanged.RemoveDynamic(this, &USuqsWaypointSubsystem::OnWaypointEnabledChanged);
		Waypoint->OnWaypointIsCurrentChanged.RemoveDynamic(this, &USuqsWaypointSubsystem::OnWaypointIsCurrentChanged);
		
	}
	
}

USuqsWaypointComponent* USuqsWaypointSubsystem::GetWaypoint(const FName& QuestID,
                                                            const FName& TaskID,
                                                            bool bOnlyEnabled)
{
	const auto pList = WaypointsByQuest.Find(QuestID);
	if (pList)
	{
		for (auto W : *pList)
		{
			if (W->GetTaskID() == TaskID &&
				(!bOnlyEnabled || W->IsEnabled()))
			{
				// List is pre-sorted by sequence index & grouped by task so this is the first
				return W;
			}
		}
	}
	return nullptr;
}

bool USuqsWaypointSubsystem::GetWaypoints(const FName& QuestID,
                                          const FName& TaskID,
                                          bool bOnlyEnabled,
                                          TArray<USuqsWaypointComponent*>& OutWaypoints)
{
	const auto pList = WaypointsByQuest.Find(QuestID);
	bool bAnyFound = false;
	if (pList)
	{
		bool bFoundTask = false;
		for (auto W : *pList)
		{
			if (W->GetTaskID() == TaskID)
			{
				bFoundTask = true;
				if (!bOnlyEnabled || W->IsEnabled())
				{
					// Waypoints are pre-sorted by SequenceIndex
					OutWaypoints.Add(W);
					// we won't use OutWayPoints.Num() to determine if we found anything, because we're not clearing the list
					// to give callers the flexibility of appending to an existing list if they want
					bAnyFound = true;
				}
			}
			else if (bFoundTask)
			{
				// Tasks are grouped in the list already so we can process in sequence
				// Once we exit the task we know we're done
				break;
			}
		}
	}
	return bAnyFound;
}

void USuqsWaypointSubsystem::SetProgression(USuqsProgression* Prog)
{
	if (Progression.IsValid())
		Progression->OnProgressionLoaded.RemoveDynamic(this, &USuqsWaypointSubsystem::OnProgressionLoaded);
	
	Progression = Prog;
	
	if (IsValid(Prog))
		Prog->OnProgressionLoaded.AddDynamic(this, &USuqsWaypointSubsystem::OnProgressionLoaded);
}

void USuqsWaypointSubsystem::OnProgressionLoaded(USuqsProgression* Prog)
{
	if (Progression.Get() == Prog)
	{
		// Refresh all waypoints currently loaded
		for (auto Pair : WaypointsByQuest)
		{
			auto Waypoints = Pair.Value;
			for (auto W : Waypoints)
			{
				W->SetIsCurrent(Progression->IsTaskRelevant(W->GetQuestID(), W->GetTaskID()));
			}
		}
	}
}

void USuqsWaypointSubsystem::OnWaypointMoved(USuqsWaypointComponent* Waypoint)
{
	// Just relay
	OnAnyWaypointMoved.Broadcast(Waypoint);
}

void USuqsWaypointSubsystem::OnWaypointEnabledChanged(USuqsWaypointComponent* Waypoint)
{
	// Just relay
	OnAnyWaypointEnabledChanged.Broadcast(Waypoint);
}

void USuqsWaypointSubsystem::OnWaypointIsCurrentChanged(USuqsWaypointComponent* Waypoint)
{
	// Just relay
	OnAnyWaypointIsCurrentChanged.Broadcast(Waypoint);
}
