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
	/// Unique identifier of the task
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FName Identifier;

	/// The player-visible text of the task
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FText Title;

	/// The number of times this task needs to be completed to be checked off
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	int TargetNumber = 1;

	/// An optional time limit for the task
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	float TimeLimit = 0;

};

/// A sub-objective of a quest. These objectives are usually performed in order but may be non-sequential
/// They can also be mandatory or optional. They comprise multiple tasks, which are the actual things the player must
/// complete.
USTRUCT(BlueprintType)
struct SUQS_API FSuqsObjective
{
	GENERATED_BODY()
public:
	/// Identifier for this objective
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	FName Identifier;

	/// The title of this objective 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	FText Title;

	/// Additional description to append to the quest description when this quest is active (and uncompleted)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FText DescriptionWhenActive;
	
	/// Additional description to append to the quest description when this quest is completed
	/// Unlike quests, if this is blank it won't use the active description
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FText DescriptionWhenCompleted;

	/// Whether this objective is mandatory to complete the quest.
	/// NOTE: you shouldn't really have an optional objective in a sequential objective list! Use SubTasks to structure things
	/// if you need a mixture.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	bool bMandatory = true;

	/// Whether the tasks in this objective need to be completed sequentially. Default is false, since usually your
	/// objectives are sequential and your tasks are parallel (that's largely the point of the division)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Objective")
	bool bSequentialTasks = false;

	/// List of actual tasks that must be performed to complete this objective. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	TArray<FSuqsTask> Tasks;
};

/// A top-level quest. Made up of objectives which represent the multi-stage nature of a quest. Objectives are usually
/// sequential, and can themselves contain multiple tasks which can be sequential or completed in any order.
/// Quests can depend on other quests before being activated.
USTRUCT(BlueprintType)
struct SUQS_API FSuqsQuest : public FTableRowBase
{
	GENERATED_BODY()

public:

	/// Unique identifier for the quest
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

	/// Whether this quest should be automatically enabled when its prerequisites are fulfilled (deafult false)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	bool AutoEnable = false;

	/// List of quest identifiers which must be completed in order for this quest to be enabled
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	TArray<FName> PrerequiteQuests;

	/// Whether the objectives must be completed in order (default true)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	bool bSequentialObjectives = true;

	/// List of objectives involved in the quest. At least one must be mandatory.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	TArray<FSuqsObjective> Objectives;

};
