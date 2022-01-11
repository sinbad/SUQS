#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SuqsWaypointSubsystem.generated.h"

class USuqsWaypointComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaypointMoved, USuqsWaypointComponent*, Waypoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaypointEnabledChanged, USuqsWaypointComponent*, Waypoint);

/**
 * SUQS subsystem to manage the presence of quest waypoints in the current level.
 * This isn't exposed publicly, we just use it internally to register & manage waypoints, which
 * are instances of USuqsWaypointComponent.
 * Public exposure of these details is on USuqsProgression, which will listen to the events this class raises.
 */
UCLASS()
class USuqsWaypointSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	// Indexed by QuestID, then a list grouped by TaskID, ordered within that by SequenceIndex
	TMap<FName, TArray<USuqsWaypointComponent*>> WaypointsByQuest;
	
public:

	/// Raised when a currently registered waypoint moves
	UPROPERTY(BlueprintAssignable)
	FOnWaypointMoved OnWaypointMoved;
	/// Raised when a currently registered waypoint is enabled / disabled
	UPROPERTY(BlueprintAssignable)
	FOnWaypointEnabledChanged OnWaypointEnabledChanged;
	
	void RegisterWaypoint(USuqsWaypointComponent* Waypoint);
	void UnregisterWaypoint(USuqsWaypointComponent* Waypoint);

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
	
	void NotifyWaypointMoved(USuqsWaypointComponent* Waypoint) const;
	void NotifyWaypointEnabledChanged(USuqsWaypointComponent* Waypoint) const;
};
