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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Task")
	FName Identifier;

	/// The player-visible text of the task
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Task")
	FText Title;

	/// Whether this task is mandatory to complete the objective
	/// NOTE: Non-mandatory objectives are always available even in sequential task lists (they are out of sequence)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	bool bMandatory = true;


	/// The number of times this task needs to be completed to be checked off
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Task")
	int TargetNumber = 1;

	/// An optional time limit for the task. Once a task is started, it will be auto-failed if this time limit expires
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Task")
	float TimeLimit = 0;

	/// An optional time delay so that after this task is completed / failed, this much time must pass before the
	/// knock-on effects of that are resolved (activating next task etc)
	/// If set to >= 0 this will override any defaults set via USuqsProgression::SetDefaultProgressionTimeDelays
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Task")
	float ProgressionDelay = -1;

	/// If set, when this task is completed or failed, the knock-on effects of this will not be resolved until
	/// the Gate of this name is opened on USuqsProgression. This allows you to control precisely when the next task
	/// after this one is activated.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Task")
	FName ProgressionGate;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	FName Identifier;

	/// The title of this objective 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	FText Title;

	/// Additional description to append to the quest description when this quest is active (and uncompleted)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	FText DescriptionWhenActive;
	
	/// Additional description to append to the quest description when this quest is completed
	/// Unlike quests, if this is blank it won't use the active description
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	FText DescriptionWhenCompleted;

	/// Whether the tasks in this objective need to be completed sequentially. Default is false, since 
	/// objectives are sequential and your tasks are often parallel, but you can change this.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	bool bSequentialTasks = false;

	/// The number of mandatory tasks under this objective which must complete for the objective to be complete
	/// Default is -1, which means ALL mandatory tasks. Change this to another number to only require a certain
	/// number of tasks to complete. Only used if bSequentialTasks = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	int NumberOfMandatoryTasksRequired = -1;

	/// Quest branch which this objective appears on. To support branching quests, if an objective is assigned
	/// to a branch, that objective is only active if that quest branch is active. No branches
	/// are active by default so you have to activate quest branches to activate these objectives.
	/// Objectives are always still sequential but essentially depend on the previous active objective (taking into account branches)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	FName Branch;
	
	/// An optional time delay so that after this objective is completed / failed, this much time must pass before the
	/// knock-on effects of that are resolved (activating next objective, completing/failing the quest etc)
	/// If set to >= 0 this will override any defaults set via USuqsProgression::SetDefaultProgressionTimeDelays
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	float ProgressionDelay = -1;

	/// If set, when this objective is completed or failed, the knock-on effects of this will not be resolved until
	/// the Gate of this name is opened on USuqsProgression. This allows you to control precisely when the next steps
	/// after completion / failure happen
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	FName ProgressionGate;
	
	/// List of actual tasks that must be performed to complete this objective. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FName Identifier;

	/// Whether this quest is intended for the player to see, or whether it's just an internal state tracker
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	bool bPlayerVisible = true;
	
	/// Summary title of the quest
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FText Title;

	/// Detailed description of the quest while active
	/// Note: this can be appended to by objectives
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FText DescriptionWhenActive;

	/// Detailed description of the quest when complete
	/// If blank, DescriptionWhenActive will be used always
	/// Note: this can be appended to by objectives
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FText DescriptionWhenCompleted;

	/// Whether this quest should be automatically accepted when its prerequisites are fulfilled (default false)
	/// When false, this quest can only be accepted by calling USuqsProgression::AcceptQuest
	/// AutoAccept=true with no prerequisites does nothing.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	bool AutoAccept = false;

	/// List of quest identifiers which must be completed successfully in order for this quest to be accepted
	/// Only does something when AutoAccept is enabled. This allows you to activate quest chains based on completion.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	TArray<FName> PrerequisiteQuests;

	/// List of quest identifiers which must have failed in order for this quest to be accepted
	/// Only does something when AutoAccept is enabled. This allows you to activate quest chains based on failure.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	TArray<FName> PrerequisiteQuestFailures;	

	/// List of quest branches which are active by default when the quest is accepted
	/// This is equivalent to calling SetBranchActive on the quest immediately after accepting it
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	TArray<FName> DefaultActiveBranches;

	/// An optional time delay so that after this quest is completed / failed, this much time must pass before the
	/// knock-on effects of that are resolved (activating a dependent quest, removing it from the active quests)
	/// If set to >= 0 this will override any defaults set via USuqsProgression::SetDefaultProgressionTimeDelays
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	float ProgressionDelay = -1;

	/// If set, when this quest is completed or failed, the knock-on effects of this will not be resolved until
	/// the Gate of this name is opened on USuqsProgression. This allows you to control precisely when the quest disappears
	/// from the active list, or when quests that come after it are accepted, instead of immediately
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	FName ProgressionGate;
	
	/// List of objectives involved in the quest. They are all sequential, and mandatory, but may be hidden to provide some branching
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	TArray<FSuqsObjective> Objectives;

	/// Attempt to get an objective by its identifier
	const FSuqsObjective* FindObjective(const FName& Identifier) const;
};
