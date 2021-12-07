#pragma once

#include "CoreMinimal.h"

#include "SuqsObjectiveState.h"
#include "UObject/Object.h"
#include "SuqsTaskState.generated.h"

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
UCLASS()
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
	FSuqsProgressionBarrier ProgressionBarrier;
	
	const FSuqsTask* TaskDefinition;
	TWeakObjectPtr<USuqsObjectiveState> ParentObjective;
	TWeakObjectPtr<USuqsProgression> Progression;

	
	void Initialise(const FSuqsTask* TaskDef, USuqsObjectiveState* ObjState, USuqsProgression* Root);
	void Tick(float DeltaTime);
	void ChangeStatus(ESuqsTaskStatus NewStatus);
	void QueueParentStatusChangeNotification();
	bool IsProgressionBlockedOn(ESuqsProgressionBarrierCondition Barrier) const;
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

	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FName& GetIdentifier() const { return TaskDefinition->Identifier; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsMandatory() { return TaskDefinition->bMandatory; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsTimeLimited() const { return TaskDefinition->TimeLimit > 0; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetTimeLimit() const { return TaskDefinition->TimeLimit; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FText& GetTitle() const { return TaskDefinition->Title; }
	/// The target number of things to be achieved
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetTargetNumber() const { return TaskDefinition->TargetNumber; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USuqsObjectiveState* GetParentObjective() const { return ParentObjective.Get(); }

	/// Fail this task
	UFUNCTION(BlueprintCallable)
	void Fail();
	/// Complete this task (setting number to target number automatically)
	UFUNCTION(BlueprintCallable)
	bool Complete();
	
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

	/// Reset the progress on this task back to the initial state
	UFUNCTION(BlueprintCallable)
    void Reset();
	
	void NotifyGateOpened(const FName& GateName);
};
