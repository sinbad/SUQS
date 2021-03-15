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


	const FSuqsTask* TaskDefinition;
	TWeakObjectPtr<USuqsObjectiveState> ParentObjective;
	TWeakObjectPtr<USuqsPlayState> PlayState;

	
	void Initialise(const FSuqsTask* TaskDef, USuqsObjectiveState* ObjState, USuqsPlayState* Root);
	void Tick(float DeltaTime);
	void ChangeStatus(ESuqsTaskStatus NewStatus);
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
	const FText& GetTitle() { return TaskDefinition->Title; }
	/// The target number of things to be achieved
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetTargetNumber() { return TaskDefinition->TargetNumber; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USuqsObjectiveState* GetParentObjective() const { return ParentObjective.Get(); }

	/// Fail this task
	UFUNCTION(BlueprintCallable)
	void Fail();
	/// Complete this task (setting number to target number automatically)
	UFUNCTION(BlueprintCallable)
	bool Complete();
	/// Advance the number associated with progress on this quest. If it reaches the target number or more, it will automatically complete
	UFUNCTION(BlueprintCallable)
	void Progress(int Delta);

	/// Return whether a task is neither complete nor failed 
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsIncomplete() const { return Status != ESuqsTaskStatus::Completed && Status != ESuqsTaskStatus::Failed; }

	/// Reset the progress on this task back to the initial state
	UFUNCTION(BlueprintCallable)
    void Reset();
};
