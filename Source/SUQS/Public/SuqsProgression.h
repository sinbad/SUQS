#pragma once

#include "CoreMinimal.h"
#include "SuqsQuest.h"
#include "SuqsQuestState.h"
#include "Engine/DataTable.h"
#include "UObject/Object.h"
#include "SuqsSaveData.h"
#include "SuqsTaskState.h"
#include "SuqsProgression.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskUpdated, USuqsTaskState*, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskCompleted, USuqsTaskState*, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskFailed, USuqsTaskState*, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, USuqsObjectiveState*, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveFailed, USuqsObjectiveState*, Objective);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActiveQuestsListChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, USuqsQuestState*, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, USuqsQuestState*, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestAccepted, USuqsQuestState*, Quest);

// C++ only because of non-const struct
DECLARE_DELEGATE_TwoParams(FOnPreLoad, USuqsProgression*, FSuqsSaveData&);

/**
 * Progression holds all the state relating to all quests and their objectives/tasks for a single player.
 * Add this somewhere that's useful to you, e.g. your PlayerState or GameInstance.
 * And of course, you'll want to include it in your save games.
 */
UCLASS(BlueprintType, Blueprintable)
class SUQS_API USuqsProgression : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TArray<UDataTable*> QuestDataTables;

	/// Unified quest defs, combined from all entries in QuestDataTables
	UPROPERTY()
	TMap<FName, FSuqsQuest> QuestDefinitions;

	/// Map of active quests
	UPROPERTY()
	TMap<FName, USuqsQuestState*> ActiveQuests;

	/// Archive of completed / failed quests
	UPROPERTY()
	TMap<FName, USuqsQuestState*> QuestArchive;

	TArray<FName> GlobalActiveBranches;
	TSet<FName> OpenGates;
	// Name of quest completed -> names of other quests that depend on its completion
	TMultiMap<FName, FName> QuestCompletionDeps;
	// Name of quest completed -> names of other quests that depend on its failure
	TMultiMap<FName, FName> QuestFailureDeps;

	bool bSuppressEvents = false;
	float DefaultQuestProgressionTimeDelay = 0;
	float DefaultObjectiveProgressionTimeDelay = 0;
	float DefaultTaskProgressionTimeDelay = 0;
	

	USuqsQuestState* FindQuestState(const FName& QuestID);
	const USuqsQuestState* FindQuestState(const FName& QuestID) const;
	USuqsTaskState* FindTaskStatus(const FName& QuestID, const FName& TaskID);

	void RebuildAllQuestData();
	void AutoAcceptQuests(const FName& FinishedQuestID, bool bFailed);
	static void SaveToData(TMap<FName, USuqsQuestState*> Quests, FSuqsSaveData& Data);

public:

	/// Initialise this progress instance with quest definitions contained in the passed in tables
	/// You should only call this once at startup. Calling it again will reset all progress data.
	/// You must also call this BEFORE calling any other function
	UFUNCTION(BlueprintCallable)
	void InitWithQuestDataTables(TArray<UDataTable*> Tables);

	/// Initialise this progress instance with quest definitions contained in all datatables found in a given content location.
	/// Path must be of the form "/Game/Sub/Folder/Within/Content". Will recurse into subdirectories.
	/// You must also call this BEFORE calling any other function
	UFUNCTION(BlueprintCallable)
    void InitWithQuestDataTablesInPath(const FString& Path);

	/// Initialise this progress instance with quest definitions contained in all datatables found in a given content locations.
	/// Paths must be of the form "/Game/Sub/Folder/Within/Content". Will recurse into subdirectories.
	/// You must also call this BEFORE calling any other function
	UFUNCTION(BlueprintCallable)
    void InitWithQuestDataTablesInPaths(const TArray<FString>& Paths);

	/**
	 * Change the default time delays between completing / failing a quest item, and the knock-on effects of that
	 * (the next task/objective/quest being activated).
	 * Where one completion rolls into another, the time delays are in serial. Therefore when completing the last
	 * task in an objective, the task delay will tick by before the objective is completed, then the objective delay
	 * will need to tick before the quest state is updated. Finally if that completes the whole quest, the quest delay
	 * will have to tick before any dependent quests are activated.
	 * 
	 * @param QuestDelay The time between a quest being completed/failed, and the effects on dependent quests (default 2)
	 * @param ObjectiveDelay The time between an objective being completed/failed, and the effect on the parent quest and following objectives (default 0)
	 * @param TaskDelay The time between a task being completed/failed, and dependent effects (next task, objective complete etc) (default 2)
	 */
	UFUNCTION(BlueprintCallable)
	void SetDefaultProgressionTimeDelays(float QuestDelay, float ObjectiveDelay, float TaskDelay);

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
	/// Fired when a quest has been accepted for the first time
	UPROPERTY(BlueprintAssignable)
	FOnQuestAccepted OnQuestAccepted;
	/// Fired whenever the active quest list changes
	UPROPERTY(BlueprintAssignable)
	FOnActiveQuestsListChanged OnActiveQuestsListChanged;

	/// Use this from C++ to receive access to the loaded quest data before it's applied to this progression
	/// You can therefore change the quest data if you need to adapt it due to quest changes
	FOnPreLoad OnPreLoad;

	
	/**
	 * Return the quest definitions available. These are separate to the list of accepted quests or quest archive and
	 * represents all of the quests available.
	 * @param bForceRebuild If true, force the internal rebuild of quest definitions. Only needed if you have changed the
	 *   QuestDataTables property at runtime after using other methods on this instance, which is highly discouraged.
	 * @return The quest definitions
	 */
	UFUNCTION(BlueprintCallable)
	const TMap<FName, FSuqsQuest>& GetQuestDefinitions(bool bForceRebuild = false);

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
	/// Note: this does NOT undo prior auto-acceptance of other quests which were dependent on this quest's completion/failure
	/// If you need those other quests to reset or be removed if undoing completion/failure, you must do it manually.
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
	 * @param QuestID The ID of the quest. If None, will scan all active quests and fail any task with TaskIdentifier
	 * @param TaskIdentifier The identifier of the task within the quest
	 */
	UFUNCTION(BlueprintCallable)
    void FailTask(FName QuestID, FName TaskIdentifier);

	/**
	 * Fully complete a task. If this is the last mandatory task in an objective, also completes the objective, and
	 * cascades upwards to the quest if that's the last mandatory objective.
	 * @param QuestID The ID of the quest. If None, will scan all active quests and complete any task with TaskIdentifier
	 * @param TaskIdentifier The identifier of the task within the quest (required)
	 * @returns Whether the task was successfully completed
	 */
	UFUNCTION(BlueprintCallable)
	bool CompleteTask(FName QuestID, FName TaskIdentifier);

	/**
	 * Increment task progress. Increases the number value on a task, clamping it to the min/max numbers in the quest
	 * definition. If this increment takes the task number to the target, it completes the task as per CompleteTask.
	 * @param QuestID The ID of the quest. If None, will scan all active quests and progress any task with TaskIdentifier
	 * @param TaskIdentifier The identifier of the task within the quest
	 * @param Delta The change to make to the number on the task
	 * @returns The number of "things" outstanding on the task after progress was applied
	 */
	UFUNCTION(BlueprintCallable)
	int ProgressTask(FName QuestID, FName TaskIdentifier, int Delta);

	/// Get the current objective for a given quest
	UFUNCTION(BlueprintCallable)
	USuqsObjectiveState* GetCurrentObjective(FName QuestID) const;

	/// Return whether a given objective is incomplete ie not failed or completed. For more information, retrieve the objective itself
	UFUNCTION(BlueprintCallable)
    bool IsObjectiveIncomplete(FName QuestID, FName ObjectiveID) const;

	/// Return whether a given objective is completed. For more information, retrieve the objective itself
	UFUNCTION(BlueprintCallable)
    bool IsObjectiveCompleted(FName QuestID, FName ObjectiveID) const;

	/// Return whether the objective has failed. For more information, retrieve the objective itself
	UFUNCTION(BlueprintCallable)
    bool IsObjectiveFailed(FName QuestID, FName ObjectiveID) const;

	/// Reset a single objective in the quest, rather than the entire quest
	UFUNCTION(BlueprintCallable)
	void ResetObjective(FName QuestID, FName ObjectiveID);

	/// Get the next mandatory task for a given quest.
	/// If the objective for this quest only requires ONE of a number of tasks to be completed, this will be the first one.
	/// Check the current objective for more details.
	UFUNCTION(BlueprintCallable)
    USuqsTaskState* GetNextMandatoryTask(FName QuestID) const;

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

	/// Reset a single task in the quest
	UFUNCTION(BlueprintCallable)
    void ResetTask(FName QuestID, FName TaskID);

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

	/// Return a NON-MODIFIABLE list of globally active branches
	UFUNCTION(BlueprintCallable)
	const TArray<FName>& GetGlobalActiveQuestBranches() const;


	/// Set a whether a given progression gate is open. Gates are unique names which prevent automatic progression on
	/// completion / failure of an item until that gate is set open (all gates are closed by default). This can
	/// help you manually control when a task / objective / quest rolls over to the next on completion / failure, so
	/// you can tick off items but not have things move forward until you're ready.
	UFUNCTION(BlueprintCallable)
	void SetGateOpen(FName GateName, bool bOpen = true);
	
	/// Return whether a gate to progression is open.
	UFUNCTION(BlueprintCallable)
	bool IsGateOpen(FName GateName);
	
	/// Return whether the dependencies for a given quest have been met
	/// You don't usually need to call this yourself, if auto-accept is enabled on your quests. But if you
	/// want to determine it manually you can.
	UFUNCTION(BlueprintCallable)
	bool QuestDependenciesMet(const FName& QuestID);

	void RaiseTaskUpdated(USuqsTaskState* Task);
	void RaiseTaskFailed(USuqsTaskState* Task);
	void RaiseTaskCompleted(USuqsTaskState* Task);
	void RaiseObjectiveCompleted(USuqsObjectiveState* Objective);
	void RaiseObjectiveFailed(USuqsObjectiveState* Objective);
	void RaiseQuestCompleted(USuqsQuestState* Quest);
	void RaiseQuestFailed(USuqsQuestState* Quest);
	void RaiseQuestReset(USuqsQuestState* Quest);

	void ProcessQuestStatusChange(USuqsQuestState* Quest);

	const FSuqsQuest* GetQuestDefinition(const FName& QuestID);

	/// Given a task definition and status, return progression barrier information
	FSuqsProgressionBarrier GetProgressionBarrierForTask(const FSuqsTask* Task, ESuqsTaskStatus Status) const;
	/// Given a task definition and status, return progression barrier information
	FSuqsProgressionBarrier GetProgressionBarrierForObjective(const FSuqsObjective* Obj, ESuqsObjectiveStatus Status) const;
	/// Given a task definition and status, return progression barrier information
	FSuqsProgressionBarrier GetProgressionBarrierForQuest(const FSuqsQuest* Q, ESuqsQuestStatus Status) const;

	/// Standard serialisation support
	virtual void Serialize(FArchive& Ar) override;

	/// Specific load from our data holder structs, if you prefer over Serialize()
	void LoadFromData(const FSuqsSaveData& Data);
	/// Specific save to our data holder structs, if you prefer over Serialize()
	void SaveToData(FSuqsSaveData& Data) const;

	// FTickableGameObject begin
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	// FTickableGameObject end

	/**
	 * @brief 
	 * @param JsonString Quest definition in a JSON string
	 * @return A quest data table suitable for adding to USuqsProgression's QuestDataTables property
	 */
	UFUNCTION(BlueprintCallable, Category="SUQS")
	static UDataTable* MakeQuestDataTableFromJSON(const FString& JsonString);

};
