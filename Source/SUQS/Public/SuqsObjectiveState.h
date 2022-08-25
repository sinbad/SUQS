#pragma once

#include "CoreMinimal.h"

#include "SuqsQuestState.h"
#include "UObject/Object.h"
#include "SuqsObjectiveState.generated.h"


UENUM(BlueprintType)
enum class ESuqsObjectiveStatus : uint8
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
 * Objective state
 */
UCLASS(BlueprintType)
class SUQS_API USuqsObjectiveState : public UObject
{
	GENERATED_BODY()

	friend class USuqsQuestState;
protected:
	/// Whether this objective has been started, completed, failed (quick access to looking at tasks)
	UPROPERTY(BlueprintReadOnly, Category="Objective Status")
	ESuqsObjectiveStatus Status = ESuqsObjectiveStatus::NotStarted;

	/// List of detailed task status
	UPROPERTY(BlueprintReadOnly, Category="Objective Status")
	TArray<USuqsTaskState*> Tasks;
	
	const FSuqsObjective* ObjectiveDefinition;
	TWeakObjectPtr<USuqsQuestState> ParentQuest;
	TWeakObjectPtr<USuqsProgression> Progression;

	int MandatoryTasksNeededToComplete;

	
	void Initialise(const FSuqsObjective* ObjDef, USuqsQuestState* QuestState, USuqsProgression* Root);
	void Tick(float DeltaTime);
	// Private fail/complete since users should only ever call task fail/complete
	void ChangeStatus(ESuqsObjectiveStatus NewStatus);

public:
	// C++ access
	
	ESuqsObjectiveStatus GetStatus() const { return Status; }
	const TArray<USuqsTaskState*>& GetTasks() { return Tasks; }

	/// Objective identifier, which may be blank
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FName& GetIdentifier() const { return ObjectiveDefinition->Identifier; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FText& GetTitle() const { return ObjectiveDefinition->Title; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    USuqsQuestState* GetParentQuest() const { return ParentQuest.Get(); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	USuqsProgression* GetRootProgression() const { return Progression.Get(); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FName& GetBranch() const { return ObjectiveDefinition->Branch; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool AreTasksSequential() const { return ObjectiveDefinition->bSequentialTasks; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool AreAllMandatoryTasksRequired() const { return ObjectiveDefinition->NumberOfMandatoryTasksRequired == -1; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    int NumberOfMandatoryTasksRequired() const { return MandatoryTasksNeededToComplete; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetContinueOnFail() { return ObjectiveDefinition->bContinueOnFail; }

	/// Get the additional description to be added to quest description for this objective, if any
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FText& GetDescription() const;
	/// Return whether an objective is neither complete nor failed 
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsIncomplete() const { return Status != ESuqsObjectiveStatus::Completed && Status != ESuqsObjectiveStatus::Failed; }
	/// Return whether an objective is completed 
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsCompleted() const { return Status == ESuqsObjectiveStatus::Completed; }
	/// Return whether an objective is failed 
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsFailed() const { return Status == ESuqsObjectiveStatus::Failed; }
	
	/// Reset the progress on this objective back to the initial state
	UFUNCTION(BlueprintCallable)
    void Reset();
	/// Manually fail this objective. Normally you should fail a specific task, but if you call this then it will mark
	/// the current task(s) failed. Does nothing if there are no outstanding tasks
	UFUNCTION(BlueprintCallable)
    void FailOutstandingTasks();

	/// Manually complete all mandatory tasks on this this objective. You shouldn't really use this, you should complete
	/// specific tasks and let it resolve from there, but if you call this then it will mark all mandatory task(s) completed,
	/// including those previously failed.
	void CompleteAllMandatoryTasks();

	/// Get the next incomplete mandatory task required to fulfil this objective
	/// If there are multiple mandatory tasks and ordering doesn't matter, returns the first one found
	/// May return nullptr if there is nothing to do next right now (may occur due to e.g. barriers)
	UFUNCTION(BlueprintCallable)
	USuqsTaskState* GetNextMandatoryTask() const;

	/// Convenience function to get a list of all tasks which are useful to display to a player. This means
	/// all the completed or failed tasks in this objective, plus the next incomplete mandatory task(s) (multiple if non-sequential),
	/// and any optional tasks.
	UFUNCTION(BlueprintCallable)
	void GetAllRelevantTasks(TArray<USuqsTaskState*>& RelevantTasksOut) const;


	/// Get a list of tasks which are incomplete
	UFUNCTION(BlueprintCallable)
	void GetIncompleteTasks(TArray<USuqsTaskState*>& IncompleteTasksOut) const;
	/// Get a list of tasks which are completed
	UFUNCTION(BlueprintCallable)
    void GetCompletedTasks(TArray<USuqsTaskState*>& CompletedTasksOut) const;
	/// Get a list of tasks which have been failed
	UFUNCTION(BlueprintCallable)
    void GetFailedTasks(TArray<USuqsTaskState*>& FailedTasksOut) const;
	/// Return whether this objective is on an active quest branch
	UFUNCTION(BlueprintCallable)
    bool IsOnActiveBranch() const;

	
	void NotifyTaskStatusChanged(const USuqsTaskState* ChangedTaskOrNull);
	void NotifyGateOpened(const FName& GateName);
	void FinishLoad();
};
