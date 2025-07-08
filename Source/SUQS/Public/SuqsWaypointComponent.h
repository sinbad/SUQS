﻿// Copyright 2020 Old Doorways Ltd

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SuqsWaypointComponent.generated.h"

class USuqsWaypointComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSuqsOnWaypointMoved, USuqsWaypointComponent*, Waypoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSuqsOnWaypointEnabledChanged, USuqsWaypointComponent*, Waypoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSuqsOnWaypointIsCurrentChanged, USuqsWaypointComponent*, Waypoint);


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
 * In multiplayer, waypoint components are only linked to the quest on the server. Only movement is
 * replicated.
*/
UCLASS(ClassGroup=(SUQS), meta=(BlueprintSpawnableComponent))
class SUQS_API USuqsWaypointComponent : public USceneComponent
{
	GENERATED_BODY()

protected:
	/// The ID of the quest this waypoint belongs to (required)
	/// If you leave this blank in defaults, use the Initialise() function later
	UPROPERTY(EditAnywhere, BlueprintReadOnly, BlueprintGetter=GetQuestID, Category="Waypoint")
	FName QuestID;

	/// The ID of the task within the quest that this waypoint belongs to (required)
	/// If you leave this blank in defaults, use the Initialise() function later
	UPROPERTY(EditAnywhere, BlueprintReadOnly, BlueprintGetter=GetTaskID, Category="Waypoint")
	FName TaskID;
	
	/// If > 1 waypoint is registered for a task, the sequence index can imply an order (e.g. along a path)
	/// You may wish to SetEnabled(false) on waypoints that are no longer relevant
	/// If you leave this blank in defaults, use the Initialise() function later
	UPROPERTY(EditAnywhere, BlueprintReadOnly, BlueprintGetter=GetSequenceIndex, Category="Waypoint")
	uint8 SequenceIndex = 0;
	
	/// Whether this waypoint is currently enabled
	UPROPERTY(EditAnywhere, BlueprintSetter=SetEnabled, SaveGame, ReplicatedUsing=OnRep_Enabled, BlueprintGetter=IsEnabled, Category="Waypoint")
	bool bEnabled = true;

	/// Whether this waypoint should raise move events when current
	/// Default false since you don't need this if you attach this waypoint component and visual to the same actor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Waypoint")
	bool bRaiseMoveEvents = false;

	/**
	 * Whether this waypoint is "current" ie associated with a currently relevant task
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_IsCurrent, Category="Waypoint")
	bool bIsCurrent = false;

	bool bIsRegistered = false; // server only

	UFUNCTION()
	void OnRep_Enabled();

	UFUNCTION()
	void OnRep_IsCurrent();

public:
	// Sets default values for this component's properties
	USuqsWaypointComponent();

	/// Event raised when this waypoint moves, but only if bRaiseMoveEvents is true, and it's current (to avoid message spam)
	UPROPERTY(BlueprintAssignable)
	FSuqsOnWaypointMoved OnWaypointMoved;
	/// Event raised when the enabled status of this waypoint changes
	UPROPERTY(BlueprintAssignable)
	FSuqsOnWaypointEnabledChanged OnWaypointEnabledChanged;
	/// Event raised when the waypoint becomes or stops being current (associated with an active task)
	UPROPERTY(BlueprintAssignable)
	FSuqsOnWaypointEnabledChanged OnWaypointIsCurrentChanged;

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

	
	/**
	 * Late initialise this component with quest details.
	 * Note: only relevant on the server in multiplayer.
	 * @param InQuestID The ID of the quest this waypoint belongs to (required) 
	 * @param InTaskID The ID of the task this waypoint belongs to (required)
	 * @param InSequenceIndex If > 1 waypoint is registered for a task, the sequence index can imply an order (e.g. along a path)
	 */
	UFUNCTION(BlueprintCallable)
	virtual void Initialise(FName InQuestID, FName InTaskID, uint8 InSequenceIndex = 0);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/// Internal use only
	virtual void SetIsCurrent(bool bNewIsCurrent);
	bool IsCurrent() const { return bIsCurrent; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void Register();
	virtual void Unregister();

	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;

	virtual void OnIsCurrentChanged();
	virtual void OnIsEnabledChanged();
};
