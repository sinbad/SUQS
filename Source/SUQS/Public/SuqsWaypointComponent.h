// Copyright 2020 Old Doorways Ltd

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SuqsWaypointComponent.generated.h"


/**
 * Component which represents a world waypoint for a quest task. They are loosely associated with quest data but at
 * at runtime when a level is loaded, any waypoints in the level will be registered and made available for querying.
 * Add a component of this type to your actors to hint to the player where quests can be progressed.
 * 
 * Every waypoint must be associated with a QuestID and TaskID. You can have more than one waypoint
 * for the same task, if for example you can do any of a number of things, or there are multiple places in the
 * world to achieve a task. You can also specify a sequence if those multiple waypoints should be in a sequence
 * within a task (such as a path - consider splitting the task if the waypoint sequence involves different actions).
 * 
 * Waypoints can be enabled/disabled, and move at runtime.
 *
 * See the USuqsProgression class for details on how to query for waypoints and to get events about waypoint changes.
*/
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SUQS_API USuqsWaypointComponent : public USceneComponent
{
	GENERATED_BODY()

protected:
	/// The ID of the quest this waypoint belongs to (required)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, BlueprintGetter=GetQuestID)
	FName QuestID;

	/// The ID of the task within the quest that this waypoint belongs to (required)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, BlueprintGetter=GetTaskID)
	FName TaskID;
	
	/// If > 1 waypoint is registered for a task, the sequence index can imply an order (e.g. along a path)
	/// You may wish to SetEnabled(false) on waypoints that are no longer relevant
	UPROPERTY(EditAnywhere, BlueprintReadOnly, BlueprintGetter=GetSequenceIndex)
	uint8 SequenceIndex = 0;
	
	/// Whether this waypoint is currently enabled
	UPROPERTY(EditAnywhere, BlueprintSetter=SetEnabled, SaveGame, BlueprintGetter=IsEnabled)
	bool bEnabled = true;


	/**
	 * Events will only be raised when requested of this waypoint; this avoids movement events for tasks that
	 * aren't currently of interest swamping the system
	 */
	UPROPERTY(SaveGame)
	bool bEventsEnabled;
public:
	// Sets default values for this component's properties
	USuqsWaypointComponent();

	UFUNCTION(BlueprintCallable)
	virtual FName GetQuestID() const { return QuestID; }
	UFUNCTION(BlueprintCallable)
	virtual FName GetTaskID() const { return TaskID; }
	UFUNCTION(BlueprintCallable)
	virtual uint8 GetSequenceIndex() const { return SequenceIndex; }
	UFUNCTION(BlueprintCallable)
	virtual bool IsEnabled() const { return bEnabled; }

	UFUNCTION(BlueprintCallable)
	virtual void SetEnabled(bool bNewEnabled);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/// Internal use only
	virtual void SetEventsEnabled(bool bNewEnabled);
	bool GetEventsEnabled() const { return bEventsEnabled; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
};
