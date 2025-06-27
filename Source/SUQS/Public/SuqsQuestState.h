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

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ESuqsResolveBarrierCondition : uint8
{
	None       = 0 UMETA(Hidden),
	/// Resolve won't occur until time passes
	Time       = (1 << 0),
	/// Resolve won't occur until a gate is opened (on Quest)
	Gate       = (1 << 1),
	/// Resolve won't occur until Resolve[Quest/Task] is called
	Explicit   = (1 << 2),

};
ENUM_CLASS_FLAGS(ESuqsResolveBarrierCondition);

USTRUCT(BlueprintType)
struct FSuqsResolveBarrier
{
	GENERATED_BODY()

	/// Bitflags identifying the conditional barriers to progression
	UPROPERTY(BlueprintReadOnly, meta = (Bitmask, BitmaskEnum = "/Script/SUQS.ESuqsResolveBarrierCondition"))
	int32 Conditions;

	/// The time remaining if the barrier includes time
	UPROPERTY(BlueprintReadOnly)
	float TimeRemaining;

	/// The gate that progression is blocked by, if barrier includes it
	UPROPERTY(BlueprintReadOnly)
	FName Gate;

	/// Whether explicit permission has been given to resolve
	UPROPERTY(BlueprintReadOnly)
	bool bGrantedExplicitly;
	
	/// Flag indicating whether this barrier is still pending resolution
	UPROPERTY(BlueprintReadOnly)
	bool bPending;

	FSuqsResolveBarrier() :
		Conditions(0),
		TimeRemaining(0),
		Gate(FName()),
		bGrantedExplicitly(false),
		bPending(false)
	{
	}

	FSuqsResolveBarrier(int32 InBarriers, float InTimeRemaining, const FName& InGate, bool bInGrantedExplicitly, bool bInPending)
		: Conditions(InBarriers),
		  TimeRemaining(InTimeRemaining),
		  Gate(InGate),
		  bGrantedExplicitly(bInGrantedExplicitly),
		  bPending(bInPending)
	{
	}

	::FSuqsResolveBarrier& operator=(const FSuqsResolveBarrierStateData& B)
	{
		Conditions = B.Conditions;
		TimeRemaining = B.TimeRemaining;
		Gate = FName(B.Gate);
		bGrantedExplicitly = B.bGrantedExplicitly;
		bPending = B.bPending;
		return *this;
	}

	friend bool operator==(const FSuqsResolveBarrier& A, const FSuqsResolveBarrier& B)
	{
		return A.Conditions == B.Conditions
			&& A.TimeRemaining == B.TimeRemaining
			&& A.Gate == B.Gate
			&& A.bGrantedExplicitly == B.bGrantedExplicitly
			&& A.bPending == B.bPending;
	}

	friend bool operator!=(const FSuqsResolveBarrier& A, const FSuqsResolveBarrier& B)
	{
		return !(A == B);
	}

	FSuqsResolveBarrier(const FSuqsResolveBarrierStateData& B)
	{
		*this = B;
	}

	
};

/**
 * Quest state
 */
UCLASS(BlueprintType)
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

	/// A barrier is set when status changes but parent hasn't been notified yet
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	FSuqsResolveBarrier ResolveBarrier;

	UPROPERTY()
	TMap<FName, USuqsTaskState*> FastTaskLookup;

	// Pointer to quest definition, for convenience (this is static data)
	const FSuqsQuest* QuestDefinition;
	TWeakObjectPtr<USuqsProgression> Progression;

	bool bSuppressObjectiveChangeEvent = false;

	// Whether we need to format the text is calculated at startup
	bool bTitleNeedsFormatting;
	bool bActiveDescriptionNeedsFormatting;
	bool bCompletedDescriptionNeedsFormatting;

	// When loading, we ignore some validation
	bool bIsLoading = false;

	void Initialise(const FSuqsQuest* Def, USuqsProgression* Root);
	void Tick(float DeltaTime);
	void ChangeStatus(ESuqsQuestStatus NewStatus);
	void QueueStatusChangeNotification();
	bool IsResolveBlockedOn(ESuqsResolveBarrierCondition Barrier) const;
	void MaybeNotifyStatusChange();

	bool TitleNeedsFormatting() const;
	bool DescriptionNeedsFormatting() const;
	
public:
	ESuqsQuestStatus GetStatus() const { return Status; }
	/// Return the list of ALL objectives. If you only want active objectives (branching), use GetActiveObjectives
	const TArray<USuqsObjectiveState*>& GetObjectives() const { return Objectives; }
	const TArray<FName>& GetActiveBranches() const { return ActiveBranches; }
	const FSuqsResolveBarrier& GetResolveBarrier() const { return ResolveBarrier; }

	/// Get the unique quest identifier
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FName& GetIdentifier() const { return QuestDefinition->Identifier; }
	/// Get the quest title
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FText GetTitle() const;
	/// Get quest labels
	const TArray<FName>& GetLabels() const;

	/// Get whether the underlying quest is player-visible
	bool IsPlayerVisible() const;


	/// Get the current description for this quest (just the top-level description)
	/// For any additional objective description, see GetCurrentObjective()->GetDescription();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FText GetDescription() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	USuqsProgression* GetRootProgression() const { return Progression.Get(); }
	
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
	 * Resolve the outcome of a completed/failed task; activate the next task, or complete/fail the quest if it's the last.
	 * You do not normally need to call this, tasks resolve automatically on completion/failure by default. However if
	 * the task definition sets "ResolveAutomatically" to false then you have to call this to resolve it.
	 * Has no effect on tasks which are incomplete.
	 * @param TaskID The identifier of the task within the quest (required)
	 */
	UFUNCTION(BlueprintCallable)
	void ResolveTask(FName TaskID);
	
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

	/**
	 * Directly set the current completed number on a specific task. An alternative to the delta version ProgressTask. 
	 * @param TaskID The identifier of the task within the quest
	 * @param Number The number of completed items to set the task to
	 */
	void SetTaskNumberCompleted(FName TaskID, int Number);

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

	/// Return whether this quest is completed/failed but is blocked from resolving because of an unfulfilled condition
	UFUNCTION(BlueprintCallable)
	bool IsResolveBlocked() const;
	
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
	/// May return nullptr if there is nothing to do next right now (may occur due to e.g. barriers)
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

	/// Return whether a task is "Relevant" i.e. current and incomplete
	UFUNCTION(BlueprintCallable)
	bool IsTaskRelevant(const FName& TaskID) const;
	
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

	/// Resolve the outcome of a completed/failed quest; move it to the quest archive and process the knock-on
	/// effects such as activating dependent quests.
	/// You do not normally need to call this, quests resolve automatically on completion/failure by default. However if
	/// the quest definition sets "ResolveAutomatically" to false then you have to call this to resolve it.
	/// Has no effect on quests which are incomplete.
	UFUNCTION(BlueprintCallable)
	void Resolve();

	/// Find a task with the given identifier in this quest
	UFUNCTION(BlueprintCallable)
	USuqsTaskState* GetTask(const FName& TaskID) const;

	void NotifyObjectiveStatusChanged();

	void OverrideStatus(ESuqsQuestStatus OverrideStatus);
	void NotifyGateOpened(const FName& GateName);
	// Manually override the barrier data
	void SetResolveBarrier(const FSuqsResolveBarrierStateData& Barrier);
	
	void StartLoad();
	void FinishLoad();
	bool IsLoading() const { return bIsLoading; }
};
