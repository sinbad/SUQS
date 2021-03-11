// Fill out your copyright notice in the Description page of Project Settings.

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

};

/// A sub-objective of a quest. These objectives are always sequential, but branching can be supported by association with quest branches
/// Objectives are mandatory if associated with no branch, or if the branch they are associated with is active in the quest.
/// They are made up of multiple tasks, which are the actual things the player must complete, and derive their status from them.
USTRUCT(BlueprintType)
struct SUQS_API FSuqsObjective
{
	GENERATED_BODY()
public:
	/// Identifier for this objective. Must be unique within the quest it belongs to.
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

	/// Whether all mandatory tasks in this objective must be completed for the objective to be complete (default true)
	/// If you set this to false, completing ONE of the mandatory tasks completes the objective (non-mandatory tasks do
	/// not complete the objective)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	bool bAllMandatoryTasksRequired = true;

	/// Quest branch which this objective appears on. To support branching quests, if an objective is assigned
	/// to a branch, that objective is only active if that quest branch is active. No branches
	/// are active by default so you have to activate quest branches to activate these objectives.
	/// Objectives are always still sequential but essentially depend on the previous active objective (taking into account branches)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	FName Branch;
	
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

	/// The unique Identifier of the quest
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
	/// When false, this quest can only be accepted by calling USuqsPlayState::AcceptQuest
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	bool AutoAccept = false;

	/// List of quest identifiers which must be completed in order for this quest to be enabled
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	TArray<FName> PrerequiteQuests;

	/// List of objectives involved in the quest. They are all sequential, and mandatory, but may be hidden to provide some branching
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	TArray<FSuqsObjective> Objectives;

	/// Attempt to get an objective by its identifier
	const FSuqsObjective* FindObjective(const FName& Identifier) const;
};
