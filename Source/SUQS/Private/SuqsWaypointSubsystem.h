#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SuqsWaypointSubsystem.generated.h"

class USuqsProgression;
class USuqsWaypointComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSuqsOnAnyWaypointMoved, USuqsWaypointComponent*, Waypoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSuqsOnAnyWaypointEnabledChanged, USuqsWaypointComponent*, Waypoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSuqsOnAnyWaypointIsCurrentChanged, USuqsWaypointComponent*, Waypoint);

/**
 * SUQS subsystem to manage the presence of quest waypoints in the current level.
 * This isn't exposed publicly, we just use it internally to register & manage waypoints, which
 * are instances of USuqsWaypointComponent.
 * Public exposure of these details is on USuqsProgression, which will listen to the events this class raises.
 * In multiplayer, only the server runs this class. Waypoint components / actors are replicated but their
 * association with quests is not.
 */
UCLASS()
class USuqsWaypointSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	// Indexed by QuestID, then a list grouped by TaskID, ordered within that by SequenceIndex
	TMap<FName, TArray<USuqsWaypointComponent*>> WaypointsByQuest;

	TWeakObjectPtr<USuqsProgression> Progression;
	
public:

	/// Raised when a currently registered waypoint moves, if it has movement events enabled and is current
	/// It's generally better to attach a visual to the same actor as the waypoint component instead, but you might need this
	UPROPERTY(BlueprintAssignable)
	FSuqsOnAnyWaypointMoved OnAnyWaypointMoved;
	/// Raised when any currently registered waypoint is enabled / disabled
	UPROPERTY(BlueprintAssignable)
	FSuqsOnAnyWaypointEnabledChanged OnAnyWaypointEnabledChanged;
	/// Raised when any currently registered waypoint becomes or stops being current (associated with an active task)
	UPROPERTY(BlueprintAssignable)
	FSuqsOnAnyWaypointEnabledChanged OnAnyWaypointIsCurrentChanged;
	
	void RegisterWaypoint(USuqsWaypointComponent* Waypoint);
	void UnregisterWaypoint(USuqsWaypointComponent* Waypoint);

	void SetProgression(USuqsProgression* Prog);

	/**
	 * @brief Get a single waypoint for a task
	 * @param QuestID The quest identifier (required)
	 * @param TaskID The task identifier (required)
	 * @param bOnlyEnabled Only consider enabled waypoints
	 * @return The first or only waypoint for this task, or null if there isn't one.
	 */
	USuqsWaypointComponent* GetWaypoint(const FName& QuestID, const FName& TaskID, bool bOnlyEnabled);
	
	/**
	 * @brief Get all waypoints for a task. If there are multiple, they will be ordered by sequence index
	 * @param QuestID The quest identifier (required)
	 * @param TaskID The task identifier (required)
	 * @param bOnlyEnabled Only consider enabled waypoints
	 * @param OutWaypoints Array to append to with waypoints (will not be cleared first) 
	 */
	bool GetWaypoints(const FName& QuestID, const FName& TaskID, bool bOnlyEnabled, TArray<USuqsWaypointComponent*>& OutWaypoints);

protected:
	UFUNCTION()
	void OnWaypointMoved(USuqsWaypointComponent* Waypoint);
	UFUNCTION()
	void OnWaypointEnabledChanged(USuqsWaypointComponent* Waypoint);
	UFUNCTION()
	void OnWaypointIsCurrentChanged(USuqsWaypointComponent* Waypoint);
	UFUNCTION()
	void OnProgressionLoaded(USuqsProgression* Progression);
};
