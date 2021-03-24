#pragma once

#include "CoreMinimal.h"

#include "SuqsQuest.h"
#include "SuqsSaveData.h"
#include "UObject/Object.h"

#include "SuqsQuestState.generated.h"


class USuqsObjectiveState;
class USuqsTaskState;

UENUM(BlueprintType)
enum class ESuqsQuestStatus : uint8
{
    /// Quest is accepted and in progress
    Incomplete = 0,
    /// All mandatory elements have been completed
    Completed = 8,
    /// This quest has been failed and cannot be progressed without being explicitly reset
    Failed = 20,
	/// Quest is not available because it's never been accepted
	/// This is never used on USuqsQuestState itself, just returned as a status when querying quests
	Unavailable = 30
};

/**
 * Quest state
 */
UCLASS()
class SUQS_API USuqsQuestState : public UObject
{
	GENERATED_BODY()

	friend class USuqsProgression;
protected:

	/// Whether this objective has been started, completed, failed (quick access to looking at tasks)
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	ESuqsQuestStatus Status = ESuqsQuestStatus::Incomplete;

	/// List of detailed objective status
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	TArray<USuqsObjectiveState*> Objectives;

	/// List of active branches, which affects which objectives will be considered
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	TArray<FName> ActiveBranches;

	/// The index of the current objective. -1 if quest completed
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	int CurrentObjectiveIndex = -1;


	UPROPERTY()
	TMap<FName, USuqsTaskState*> FastTaskLookup;

	// Pointer to quest definition, for convenience (this is static data)
	const FSuqsQuest* QuestDefinition;
	TWeakObjectPtr<USuqsProgression> Progression;

	void Initialise(const FSuqsQuest* Def, USuqsProgression* Root);
	void Tick(float DeltaTime);
	void ChangeStatus(ESuqsQuestStatus NewStatus);
	
public:
	ESuqsQuestStatus GetStatus() const { return Status; }
	/// Return the list of ALL objectives. If you only want active objectives (branching), use GetActiveObjectives
	const TArray<USuqsObjectiveState*>& GetObjectives() const { return Objectives; }
	const TArray<FName>& GetActiveBranches() const { return ActiveBranches; }

	/// Get the unique quest identifier
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FName& GetIdentifier() const { return QuestDefinition->Identifier; }
	/// Get the quest title
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FText& GetTitle() const { return QuestDefinition->Title; }
	/// Get the current description for this quest (just the top-level description)
	/// For any additional objective description, see GetCurrentObjective()->GetDescription();
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FText& GetDescription() const;
	
	/// Set an objective branch to be active in this quest. Objectives associated with this branch will then be allowed
	/// to activate.
	UFUNCTION(BlueprintCallable)
	void SetBranchActive(FName Branch, bool bActive);

	/// Reset quest branches so all are inactive
	UFUNCTION(BlueprintCallable)
	void ResetBranches();

	/// Return whether an objective branch is active or not
	UFUNCTION(BlueprintCallable)
    bool IsBranchActive(FName Branch);

	/**
	 * Fully complete a task. If this is the last mandatory task in an objective, also completes the objective, and
	 * cascades upwards to the quest if that's the last mandatory objective.
	 * @param TaskID The identifier of the task within the quest
	 * @returns Whether the task was successfully completed
	 */
	UFUNCTION(BlueprintCallable)
	bool CompleteTask(FName TaskID);

	/**
	 * Fail a task if it exists. 
	 * @param TaskID The identifier of the task within the quest
	 */
	UFUNCTION(BlueprintCallable)
	void FailTask(const FName& TaskID);

	/**
	 * Increment task progress, if it exists. Increases the number value on a task, clamping it to the min/max numbers in the quest
	 * definition. If this increment takes the task number to the target, it completes the task as per CompleteTask.
	 * @param TaskID The identifier of the task within the quest
	 * @param Delta The change to make to the number on the task
	 * @returns The number of "things" outstanding on the task after progress was applied (0 if not found)
	 */
	UFUNCTION(BlueprintCallable)
	int ProgressTask(FName TaskID, int Delta);

	/// Get the current objective on this quest. Will return null if quest is complete.
	UFUNCTION(BlueprintCallable, BlueprintPure)
	USuqsObjectiveState* GetCurrentObjective() const;

	/// Find a given objective by its identifier. Remember, Objective identifiers are optional so if you haven't
	/// given it an identifier, it won't show up here.
	UFUNCTION(BlueprintCallable)
	USuqsObjectiveState* GetObjective(const FName& ObjectiveID) const;

	/// Get a list of all the active objectives on this quest, given the active branches
	/// These objectives are in order of required completion
	UFUNCTION(BlueprintCallable)
	void GetActiveObjectives(TArray<USuqsObjectiveState*>& ActiveObjectivesOut) const;

	/// Return whether this quest is incomplete, i.e. accepted but not completed or failed. 
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsIncomplete() const { return Status == ESuqsQuestStatus::Incomplete; }

	/// Return whether this quest is completed
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsCompleted() const { return Status == ESuqsQuestStatus::Completed; }

	/// Return whether the quest has failed
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsFailed() const { return Status == ESuqsQuestStatus::Failed; }

	/// Return whether a given objective is incomplete ie not failed or completed
	UFUNCTION(BlueprintCallable)
	bool IsObjectiveIncomplete(const FName& Identifier) const;
	
	/// Return whether an objective is complete
	UFUNCTION(BlueprintCallable)
	bool IsObjectiveCompleted(const FName& ObjectiveID) const;

	/// Return whether an objective failed
	UFUNCTION(BlueprintCallable)
    bool IsObjectiveFailed(const FName& ObjectiveID) const;

	/// Reset a single objective in the quest, rather than the entire quest
	UFUNCTION(BlueprintCallable)
    void ResetObjective(FName ObjectiveID);

	/// Get the next mandatory task for this quest
	/// If the objective for this quest only requires ONE of a number of tasks to be completed, this will be the first one.
	/// Check the current objective for more details.
	UFUNCTION(BlueprintCallable)
	USuqsTaskState* GetNextMandatoryTask() const;

	/// Return whether a given task is incomplete ie not failed or completed
	UFUNCTION(BlueprintCallable)
	bool IsTaskIncomplete(const FName& TaskID) const;

	/// Return whether a task is complete
	UFUNCTION(BlueprintCallable)
    bool IsTaskCompleted(const FName& TaskID) const;

	/// Return whether a task failed
	UFUNCTION(BlueprintCallable)
    bool IsTaskFailed(const FName& TaskID) const;

	/// Reset a single task in the quest
	UFUNCTION(BlueprintCallable)
    void ResetTask(FName TaskID);
	
	/// Reset all the progress on this quest back to the initial state. Also resets active branches.
	UFUNCTION(BlueprintCallable)
	void Reset();

	/// Reset all the progress on objectives associated with the named quest branch
	UFUNCTION(BlueprintCallable)
    void ResetBranch(FName Branch);

	/// Manually fail this quest. Normally you should fail a specific task, but if you call this then it will mark
	/// the current task(s) failed and then fail this quest. Does nothing if the quest doesn't have outstanding tasks
	UFUNCTION(BlueprintCallable)
    void Fail();

	/// Manually complete all the required mandatory tasks on this quest. You should *really* be completing tasks
	/// instead of calling this, but if you need it, this bypasses that marks all mandatory tasks as complete
	UFUNCTION(BlueprintCallable)
	void Complete();

	/// Find a task with the given identifier in this quest
	UFUNCTION(BlueprintCallable)
	USuqsTaskState* GetTask(const FName& TaskID) const;

	void NotifyObjectiveStatusChanged();

	void OverrideStatus(ESuqsQuestStatus OverrideStatus);
};
