#pragma once

#include "CoreMinimal.h"
#include "SuqsQuest.h"
#include "SuqsQuestState.h"
#include "Engine/DataTable.h"
#include "UObject/Object.h"
#include "SuqsProgression.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSuqsProgression, Verbose, Verbose);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskUpdated, USuqsTaskState*, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskCompleted, USuqsTaskState*, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskFailed, USuqsTaskState*, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, USuqsObjectiveState*, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveFailed, USuqsObjectiveState*, Objective);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, USuqsQuestState*, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, USuqsQuestState*, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveQuestChanged, USuqsQuestState*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestAccepted, USuqsQuestState*, Quest);

/**
 * Progression holds all the state relating to all quests and their objectives/tasks for a single player.
 * Add this somewhere that's useful to you, e.g. your PlayerState or GameInstance.
 * And of course, you'll want to include it in your save games.
 */
UCLASS(BlueprintType)
class SUQS_API USuqsProgression : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
public:
	/// Provide one or more data assets which define the quests that this status is tracking against.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest Setup")
	TArray<UDataTable*> QuestDataTables;

protected:
	/// Unified quest defs, combined from all entries in QuestDataTables
	UPROPERTY()
	TMap<FName, FSuqsQuest> QuestDefinitions;

	/// Map of active quests
	UPROPERTY()
	TMap<FName, USuqsQuestState*> ActiveQuests;

	/// Archive of completed / failed quests
	UPROPERTY()
	TMap<FName, USuqsQuestState*> QuestArchive;

	UPROPERTY()
	TArray<FName> GlobalActiveBranches;

	USuqsQuestState* FindQuestState(const FName& QuestID);
	const USuqsQuestState* FindQuestState(const FName& QuestID) const;
	USuqsTaskState* FindTaskStatus(const FName& QuestID, const FName& TaskID);

	void EnsureQuestDefinitionsBuilt();
	
public:

	/// Fired when a task is completed
	UPROPERTY(BlueprintAssignable)
	FOnTaskCompleted OnTaskCompleted;
	/// Fired when a task has failed
	UPROPERTY(BlueprintAssignable)
	FOnTaskFailed OnTaskFailed;
	/// Fired when a objective is completed
	UPROPERTY(BlueprintAssignable)
	FOnObjectiveCompleted OnObjectiveCompleted;
	/// Fired when a objective has failed
	UPROPERTY(BlueprintAssignable)
	FOnObjectiveFailed OnObjectiveFailed;
	/// Fired when a quest is completed
	UPROPERTY(BlueprintAssignable)
	FOnQuestCompleted OnQuestCompleted;
	/// Fired when a quest has failed
	UPROPERTY(BlueprintAssignable)
	FOnQuestFailed OnQuestFailed;
	/// Fired when something on the detail of a task has changed (progress, time etc)
	UPROPERTY(BlueprintAssignable)
	FOnTaskUpdated OnTaskUpdated;
	/// Fired when a different quest has been made the active quest
	UPROPERTY(BlueprintAssignable)
	FOnActiveQuestChanged OnActiveQuestChanged;
	/// Fired when a quest has been accepted for the first time
	UPROPERTY(BlueprintAssignable)
	FOnQuestAccepted OnQuestAccepted;

	/// Get the overall status of a named quest
	UFUNCTION(BlueprintCallable)
	ESuqsQuestStatus GetQuestStatus(FName QuestID) const;

	/// Return whether the quest is or has been accepted for the player (may also be completed / failed)
	UFUNCTION(BlueprintCallable)
    bool IsQuestAccepted(FName QuestID) const;

	/// Return whether the quest is incomplete, i.e. accepted but not completed or failed. 
	UFUNCTION(BlueprintCallable)
    bool IsQuestIncomplete(FName QuestID) const;

	/// Return whether the quest is completed
	UFUNCTION(BlueprintCallable)
	bool IsQuestCompleted(FName QuestID) const;

	/// Return whether the quest has failed
	UFUNCTION(BlueprintCallable)
    bool IsQuestFailed(FName QuestID) const;

	/// Get a list of the IDs of accepted quests
	UFUNCTION(BlueprintCallable)
	void GetAcceptedQuestIdentifiers(TArray<FName>& AcceptedQuestIDsOut) const;

	/// Get a list of the IDs of archived quests (those that were completed or failed)
	UFUNCTION(BlueprintCallable)
    void GetArchivedQuestIdentifiers(TArray<FName>& ArchivedQuestIDsOut) const;

	/// Get the state of a quest
	UFUNCTION(BlueprintCallable)
    USuqsQuestState* GetQuest(FName QuestID);

	/// Get a list of the currently accepted quests
	UFUNCTION(BlueprintCallable)
    void GetAcceptedQuests(TArray<USuqsQuestState*>& AcceptedQuestsOut) const;

	/// Get a list of the archived quests (those that were completed or failed)
	UFUNCTION(BlueprintCallable)
    void GetArchivedQuests(TArray<USuqsQuestState*>& ArchivedQuestsOut) const;

	/**
	 * Accept a quest and track its state (if possible)
	 * Note: you don't need to do this for quests which are set to auto-accept based on the completion of other quests.
	 * However you will want to do it for events that you activate other ways, e.g. entering areas, talking to characters
	 * @param QuestID The identifier of the quest
	 * @param bResetIfFailed If this quest has failed, whether to reset it (default true)
	 * @param bResetIfComplete If this quest has been previously completed, whether to reset it. Default false (do nothing)
	 * @param bResetIfInProgress If this quest is already in progress, whether to reset it. If not, do nothing
	 * @returns Whether the quest was successfully accepted
	 */
	UFUNCTION(BlueprintCallable)
	bool AcceptQuest(FName QuestID, bool bResetIfFailed = true, bool bResetIfComplete = false, bool bResetIfInProgress = false);

	/// Reset all progress on a quest. Works whether a quest is in progress, complete or failed. Quest will remain accepted & incomplete
	UFUNCTION(BlueprintCallable)
    void ResetQuest(FName QuestID);

	/**
	 * Remove a quest from this play state entirely. This is akin to "unaccepting" a quest.
	 * @param QuestID The identifier of the quest
	 * @param bRemoveActive Whether active quests should be removed by this call (default true)
	 * @param bRemoveArchived Whether archived (failed/completed) quests should be removed (default true)
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveQuest(FName QuestID, bool bRemoveActive = true, bool bRemoveArchived = true);


	/// Manually fail a quest. You should prefer using FailTask() instead if you need to explain which specific part
	/// of a quest failed. Otherwise, this will mark all current tasks /objectives as failed.
	UFUNCTION(BlueprintCallable)
    void FailQuest(FName QuestID);

	/// Manually complete a quest. You should prefer using CompleteTask() instead of using this, since that will
	/// automatically bubble up. This will mark all mandatory tasks as complete
	UFUNCTION(BlueprintCallable)
    void CompleteQuest(FName QuestID);

	/**
	 * Mark a task as failed. If this is a mandatory task, it will fail the objective the task is attached to.
	   If the objective is mandatory, it will fail the quest. 
	 * @param QuestID The ID of the quest
	 * @param TaskIdentifier The identifier of the task within the quest
	 */
	UFUNCTION(BlueprintCallable)
    void FailTask(FName QuestID, FName TaskIdentifier);

	/**
	 * Fully complete a task. If this is the last mandatory task in an objective, also completes the objective, and
	 * cascades upwards to the quest if that's the last mandatory objective.
	 * @param QuestID The ID of the quest
	 * @param TaskIdentifier The identifier of the task within the quest
	 * @returns Whether the task was successfully completed
	 */
	UFUNCTION(BlueprintCallable)
	bool CompleteTask(FName QuestID, FName TaskIdentifier);

	/**
	 * Increment task progress. Increases the number value on a task, clamping it to the min/max numbers in the quest
	 * definition. If this increment takes the task number to the target, it completes the task as per CompleteTask.
	 * @param QuestID The ID of the quest
	 * @param TaskIdentifier The identifier of the task within the quest
	 * @param Delta The change to make to the number on the task
	 * @returns The number of "things" outstanding on the task after progress was applied
	 */
	UFUNCTION(BlueprintCallable)
	int ProgressTask(FName QuestID, FName TaskIdentifier, int Delta);

	/// Return whether a given objective is incomplete ie not failed or completed. For more information, retrieve the objective itself
	UFUNCTION(BlueprintCallable)
    bool IsObjectiveIncomplete(FName QuestID, FName ObjectiveID) const;

	/// Return whether a given objective is completed. For more information, retrieve the objective itself
	UFUNCTION(BlueprintCallable)
    bool IsObjectiveCompleted(FName QuestID, FName ObjectiveID) const;

	/// Return whether the objective has failed. For more information, retrieve the objective itself
	UFUNCTION(BlueprintCallable)
    bool IsObjectiveFailed(FName QuestID, FName ObjectiveID) const;

	/// Return whether a given task is incomplete ie not failed or completed. For more information, retrieve the task itself
	UFUNCTION(BlueprintCallable)
    bool IsTaskIncomplete(FName QuestID, FName TaskID) const;

	/// Return whether a given task is completed. For more information, retrieve the task
	UFUNCTION(BlueprintCallable)
    bool IsTaskCompleted(FName QuestID, FName TaskID) const;

	/// Return whether the task has failed. For more information, retrieve the task
	UFUNCTION(BlueprintCallable)
    bool IsTaskFailed(FName QuestID, FName TaskID) const;

	/// Shortcut to getting the whole task state for a specific quest
	UFUNCTION(BlueprintCallable)
	USuqsTaskState* GetTaskState(FName QuestID, FName TaskID) const;

	/// Set a branch to be active in a specific quest. Objectives in this quest that are associated with this
	/// branch will then be allowed to activate.
	UFUNCTION(BlueprintCallable)
    void SetQuestBranchActive(FName QuestID, FName Branch, bool bActive);

	/// Return whether an objective branch is active or not for a given quest
	UFUNCTION(BlueprintCallable)
    bool IsQuestBranchActive(FName QuestID, FName Branch);

	/// Set a branch to be active globally for ALL quests. For all quests, objectives associated with that branch will be active.
	/// This setting affects ALL quests, including any which are accepted after calling this
	UFUNCTION(BlueprintCallable)
    void SetGlobalQuestBranchActive(FName Branch, bool bActive);

	/// Reset the globally active quest branches so that none are active
	/// This setting affects ALL quests
	UFUNCTION(BlueprintCallable)
    void ResetGlobalQuestBranches();

	/// Return whether a branch is active globally for all quests or not
	UFUNCTION(BlueprintCallable)
    bool IsGlobalQuestBranchActive(FName Branch);

	void RaiseTaskUpdated(USuqsTaskState* Task);
	void RaiseTaskFailed(USuqsTaskState* Task);
	void RaiseTaskCompleted(USuqsTaskState* Task);
	void RaiseObjectiveCompleted(USuqsObjectiveState* Objective);
	void RaiseObjectiveFailed(USuqsObjectiveState* Objective);
	void RaiseQuestCompleted(USuqsQuestState* Quest);
	void RaiseQuestFailed(USuqsQuestState* Quest);
	void RaiseQuestReset(USuqsQuestState* Quest);


	
	// FTickableGameObject begin
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	// FTickableGameObject end

};
