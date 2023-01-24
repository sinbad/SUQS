#pragma once

#include "CoreMinimal.h"

#include "SuqsObjectiveState.h"
#include "UObject/Object.h"
#include "SuqsTaskState.generated.h"

class USuqsWaypointComponent;
UENUM(BlueprintType)
enum class ESuqsTaskStatus : uint8
{
    /// No progress has been made
    NotStarted = 0,
    /// At least one element of progress has been made
    InProgress = 4,
    /// All mandatory elements have been completed
    Completed = 8,
    /// This item has been failed and cannot be progressed without being explicitly reset
    Failed = 20
};


/**
 * Record of the state of a task in a quest objective
 */
UCLASS(BlueprintType)
class SUQS_API USuqsTaskState : public UObject
{
	GENERATED_BODY()

	friend class USuqsObjectiveState;
protected:
	/// Current number (vs target number)
	UPROPERTY(BlueprintReadOnly, Category="Task State")
	int Number;
	/// Current time remaining, if task has a time limit
	UPROPERTY(BlueprintReadOnly, Category="Task State")
	float TimeRemaining = 0;
	/// Whether this task has been started, completed, failed
	UPROPERTY(BlueprintReadOnly, Category="Task State")
	ESuqsTaskStatus Status = ESuqsTaskStatus::NotStarted;
	/// Whether we suggest that this task is hidden from the player right now
	/// This is the case for mandatory, sequential, incomplete tasks beyond the first one
	UPROPERTY(BlueprintReadOnly, Category="Task State")
	bool bHidden;


	/// A barrier is set when status changes but parent hasn't been notified yet
	UPROPERTY(BlueprintReadOnly, Category="Task State")
	FSuqsResolveBarrier ResolveBarrier;
	
	const FSuqsTask* TaskDefinition;
	TWeakObjectPtr<USuqsObjectiveState> ParentObjective;
	TWeakObjectPtr<USuqsProgression> Progression;

	bool bTitleNeedsFormatting;
	
	void Initialise(const FSuqsTask* TaskDef, USuqsObjectiveState* ObjState, USuqsProgression* Root);
	void Tick(float DeltaTime);
	void ChangeStatus(ESuqsTaskStatus NewStatus, bool bIgnoreResolveBarriers = false);
	void QueueParentStatusChangeNotification(bool bIgnoreBarriers);
	bool IsResolveBlockedOn(ESuqsResolveBarrierCondition Barrier) const;
	void MaybeNotifyParentStatusChange();
public:
	// expose BP properties for C++ 
	
	/// Current number (vs target number of quest)
	int GetNumber() const { return Number; }
	/// Current time remaining, if task has a time limit
	float GetTimeRemaining() const { return TimeRemaining; }
	ESuqsTaskStatus GetStatus() const {  return Status; }
	/// Return whether this task should be hidden, e.g. because tasks are sequential in this objective
	bool GetHidden() const { return bHidden; }
	const FSuqsResolveBarrier& GetResolveBarrier() const { return ResolveBarrier; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FName& GetIdentifier() const { return TaskDefinition->Identifier; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsMandatory() const { return TaskDefinition->bMandatory; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsTimeLimited() const { return TaskDefinition->TimeLimit > 0; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetTimeLimit() const { return TaskDefinition->TimeLimit; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FText GetTitle() const;
	/// The target number of things to be achieved
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetTargetNumber() const { return TaskDefinition->TargetNumber; }
	/// Return whether or not this task has a number of things to achieve rather than just 1 discrete completion
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasTargetNumber() const { return GetTargetNumber() > 1; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USuqsObjectiveState* GetParentObjective() const { return ParentObjective.Get(); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USuqsProgression* GetRootProgression() const { return Progression.Get(); }
	
	/// Fail this task
	UFUNCTION(BlueprintCallable)
	void Fail(bool bIgnoreResolveBarriers = false);
	/// Complete this task (setting number to target number automatically)
	UFUNCTION(BlueprintCallable)
	bool Complete(bool bIgnoreResolveBarriers = false);

	/**
	 * Resolve the outcome of a completed/failed task; activate the next task, or complete/fail the quest if it's the last.
	 * You do not normally need to call this, tasks resolve automatically on completion/failure by default. However if
	 * the task definition sets "ResolveAutomatically" to false then you have to call this to resolve it.
	 * Has no effect on tasks which are incomplete.
	 * @returns Whether the task was successfully resolved
	 */
	UFUNCTION(BlueprintCallable)
	void Resolve();

	/**
	 * Advance the number associated with progress on this quest. If it reaches the target number or more, it will automatically complete
	 * @param Delta The number to change the progress by
	 * @return The number of things outstanding after the delta was applied
	 */
	UFUNCTION(BlueprintCallable)
	int Progress(int Delta);

	/// Directly change the number on this task (vs target number of quest)
	/// See also Progress()
	UFUNCTION(BlueprintCallable)
    void SetNumber(int N);

	/// Directly change the time remaining on this task
	UFUNCTION(BlueprintCallable)
    void SetTimeRemaining(float T);

	/// Directly change the barrier state
	UFUNCTION(BlueprintCallable)
	void SetResolveBarrier(const FSuqsResolveBarrier& Barrier);

	/// Get the number of "things" still left to do, will only be > 1 if TargetNumber on the task was > 1
	UFUNCTION(BlueprintCallable)
    int GetNumberOutstanding() const;
	

	/// Return whether a task is neither complete nor failed 
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsIncomplete() const { return Status != ESuqsTaskStatus::Completed && Status != ESuqsTaskStatus::Failed; }

	/// Return whether a task is completed 
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsCompleted() const { return Status == ESuqsTaskStatus::Completed; }

	/// Return whether a task is failed 
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsFailed() const { return Status == ESuqsTaskStatus::Failed; }

	/// Return whether this task is "relevant" i.e. it's on the current objective, incomplete and next in line
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsRelevant() const;

	/// Reset the progress on this task back to the initial state
	UFUNCTION(BlueprintCallable)
    void Reset();

	/// Return whether this task is completed/failed but is blocked from resolving because of an unfulfilled condition
	UFUNCTION(BlueprintCallable)
	bool IsResolveBlocked() const;
	
	void NotifyGateOpened(const FName& GateName);
	/**
	 * Return whether this task will become hidden on completion/failure or not. Reasons not to are that it's optional
	 * or the tasks are non-sequential, so stick around until the objective is completed/failed
	 */
	bool IsHiddenOnCompleteOrFail() const;

	/**
	 * @brief Get the first/next waypoint associated with this task
	 * @param bOnlyEnabled Only report enabled waypoints
	 */
	UFUNCTION(BlueprintCallable)
	USuqsWaypointComponent* GetWaypoint(bool bOnlyEnabled = true);

	/**
	 * @brief Get all world waypoint components associated with this task
	 * @param bOnlyEnabled Only report enabled waypoints
	 */
	UFUNCTION(BlueprintCallable)
	TArray<USuqsWaypointComponent*> GetWaypoints(bool bOnlyEnabled = true);
	void FinishLoad();
};
