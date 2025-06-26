#pragma once

#include "CoreMinimal.h"

#include "Engine/DataTable.h"

#include "SuqsQuest.generated.h"


/// A task is a single item of work that can be checked off, which contributes to an objective, which in turn advances a quest. 
USTRUCT(BlueprintType)
struct SUQS_API FSuqsTask
{
	GENERATED_BODY()
public:
	/// Unique identifier of the task. Must be unique within the *quest* (not just within objective)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Task")
	FName Identifier;

	/// Optional list of labels to associate with the task. Could be used for categorisation etc 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Task")
	TArray<FName> Labels;
	
	/// The player-visible text of the task
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Task")
	FText Title;

	/// Whether this task is mandatory to complete the objective
	/// NOTE: Non-mandatory objectives are always available even in sequential task lists (they are out of sequence)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Task")
	bool bMandatory = true;


	/// The number of times this task needs to be completed to be checked off
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Task")
	int TargetNumber = 1;

	/// An optional time limit for the task. Once a task is started, it will be auto-closed if this time limit expires.
	/// By default tasks fail on time limit expiry, but if you set TimeLimitCompleteOnExpiry=true then the task completes on timeout
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Task")
	float TimeLimit = 0;

	/// If this task has a TimeLimit, whether the time expiring means completion rather than failure.
	/// Useful for "wait for something" tasks that aren't handled externally
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Task")
	bool TimeLimitCompleteOnExpiry = false;

	/// If true (default), after this task is completed / failed, the knock-on effects such as activating the next task etc
	/// will happen automatically. If set to false, the knock-on effects will only happen when explicitly requested
	/// via ResolveTask
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Task")
	bool bResolveAutomatically = true;
	
	/// An optional time delay so that after this task is completed / failed, this much time must pass before the
	/// knock-on effects of that are resolved (activating next task etc)
	/// If set to >= 0 this will override any defaults set via USuqsProgression::SetDefaultProgressionTimeDelays
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Task")
	float ResolveDelay = -1;

	/// If set, when this task is completed or failed, the knock-on effects of this will not be resolved until
	/// the Gate of this name is opened on USuqsProgression. This allows you to control precisely when the next task
	/// after this one is activated.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Task")
	FName ResolveGate;

};

/// A sub-objective of a quest. These objectives are always sequential, but branching can be supported by association with quest branches
/// Objectives are mandatory if associated with no branch, or if the branch they are associated with is active in the quest.
/// They are made up of multiple tasks, which are the actual things the player must complete, and derive their status from them.
USTRUCT(BlueprintType)
struct SUQS_API FSuqsObjective
{
	GENERATED_BODY()
public:
	/// Identifier for this objective. Optional. If provided, must be unique within the quest it belongs to.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Objective")
	FName Identifier;

	/// The title of this objective 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Objective")
	FText Title;

	/// Additional description to append to the quest description when this quest is active (and uncompleted)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Objective")
	FText DescriptionWhenActive;
	
	/// Additional description to append to the quest description when this quest is completed
	/// Unlike quests, if this is blank it won't use the active description
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Objective")
	FText DescriptionWhenCompleted;

	/// Whether the tasks in this objective need to be completed sequentially. Default is false, since 
	/// objectives are sequential and your tasks are often parallel, but you can change this.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Objective")
	bool bSequentialTasks = false;

	/// The number of mandatory tasks under this objective which must complete for the objective to be complete
	/// Default is -1, which means ALL mandatory tasks. Change this to another number to only require a certain
	/// number of tasks to complete. Only used if bSequentialTasks = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Objective")
	int NumberOfMandatoryTasksRequired = -1;

	/// The default behaviour is when an objective fails, it fails the entire quest. Set this to true to allow the
	/// quest to continue on failure. For example you could make a task "stealth-optional" with this feature, or
	/// respond to failure by setting a branch active to change where the quest goes.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Objective")
	bool bContinueOnFail = false;

	/// Quest branch which this objective appears on. To support branching quests, if an objective is assigned
	/// to a branch, that objective is only active if that quest branch is active. No branches
	/// are active by default so you have to activate quest branches to activate these objectives.
	/// Objectives are always still sequential but essentially depend on the previous active objective (taking into account branches)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Objective")
	FName Branch;
	
	/// List of actual tasks that must be performed to complete this objective. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Objective")
	TArray<FSuqsTask> Tasks;

	/// Attempt to get a task by its identifier
	const FSuqsTask* FindTask(const FName& Identifier) const;
	
};

/// A top-level quest. Made up of objectives which represent the multi-stage nature of a quest. Objectives are usually
/// sequential, and can themselves contain multiple tasks which can be sequential or completed in any order.
/// Quests can depend on other quests before being activated.
/// Quests must be uniquely named (the Name property inherited from FTableRowBase). If you use more than one
/// quest table asset then there must be no name clashes between them.
/// Quest data is static; progress through the quest is tracked with USuqsStatus. This means quests / objectives / tasks cannot be
/// added at runtime. However, you can choose when to make Quests available to the player, and objectives / tasks can
/// be hidden in USuqsStatus, and activated later, which is the same as being dynamic, except you have to pre-define the
/// variants in your data. This is deliberate, it means all your quest data is visible statically which makes it easier to
/// avoid bugs introduced by code dynamically altering quests arbitrarily.
USTRUCT(BlueprintType)
struct SUQS_API FSuqsQuest : public FTableRowBase
{
	GENERATED_BODY()

public:

	/// The unique Identifier of the quest, mandatory
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	FName Identifier;

	/// Optional list of labels to associate with the quest. Could be used for categorisation etc 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	TArray<FName> Labels;

	/// Whether this quest is intended for the player to see, or whether it's just an internal state tracker
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	bool bPlayerVisible = true;
	
	/// Summary title of the quest
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	FText Title;

	/// Detailed description of the quest while active
	/// Note: this can be appended to by objectives
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	FText DescriptionWhenActive;

	/// Detailed description of the quest when complete
	/// If blank, DescriptionWhenActive will be used always
	/// Note: this can be appended to by objectives
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	FText DescriptionWhenCompleted;

	/// Whether this quest should be automatically accepted when its prerequisites are fulfilled (default false)
	/// When false, this quest can only be accepted by calling USuqsProgression::AcceptQuest
	/// AutoAccept=true with no prerequisites does nothing.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	bool AutoAccept = false;

	/// List of quest identifiers which must be completed successfully in order for this quest to be accepted
	/// Only does something when AutoAccept is enabled. This allows you to activate quest chains based on completion.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	TArray<FName> PrerequisiteQuests;

	/// List of quest identifiers which must have failed in order for this quest to be accepted
	/// Only does something when AutoAccept is enabled. This allows you to activate quest chains based on failure.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	TArray<FName> PrerequisiteQuestFailures;	

	/// List of quest branches which are active by default when the quest is accepted
	/// This is equivalent to calling SetBranchActive on the quest immediately after accepting it
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	TArray<FName> DefaultActiveBranches;

	/// If true (default), after this quest is completed / failed, the knock-on effects such as activating a dependent quest,
	/// removing this from the active quests, will happen automatically. If set to false, the knock-on effects will only
	/// happen when explicitly requested via ResolveQuest
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	bool bResolveAutomatically = true;
	
	/// An optional time delay so that after this quest is completed / failed, this much time must pass before the
	/// knock-on effects of that are resolved (activating a dependent quest, removing it from the active quests)
	/// If set to >= 0 this will override any defaults set via USuqsProgression::SetDefaultProgressionTimeDelays
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	float ResolveDelay = -1;

	/// If set, when this quest is completed or failed, the knock-on effects of this will not be resolved until
	/// the Gate of this name is opened on USuqsProgression. This allows you to control precisely when the quest disappears
	/// from the active list, or when quests that come after it are accepted, instead of immediately
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	FName ResolveGate;
	
	/// List of objectives involved in the quest. They are all sequential, and mandatory, but may be hidden to provide some branching
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	TArray<FSuqsObjective> Objectives;

	/// Attempt to get an objective by its identifier
	const FSuqsObjective* FindObjective(const FName& Identifier) const;
};
