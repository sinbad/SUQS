#pragma once
#include "CoreMinimal.h"
#include "SuqsStatusStructs.generated.h"


/// Summarises the status of a quest / objective / task
UENUM(BlueprintType)
enum class ESuqsItemStatus : uint8
{
	/// Unavailable (quest only), will not be shown to the player
	Unavailable = 0,
	/// No progress has been made on any task yet
	NotStarted = 2,
	/// At least one element of progress has been made
    InProgress = 4,
	/// All mandatory elements have been completed
    Completed = 6,
	/// This item has been failed and cannot be progressed without being explicitly reset
    Failed = 20
};

/// State of a single task, held by FSuqsObjectiveStatus
USTRUCT(BlueprintType)
struct SUQS_API FSuqsTaskState
{
	GENERATED_BODY()
public:
	/// The identifier of the objective this status refers to
	UPROPERTY(BlueprintReadOnly, Category="Task Status")
	FName Identifier;
	/// Current number (vs target number of quest)
	UPROPERTY(BlueprintReadOnly, Category="Task Status")
	int Number;
	/// Whether the task has a time limit
	UPROPERTY(BlueprintReadOnly, Category="Task Status")
	bool bTimeLimit = false;
	/// Current time remaining, if task has a time limit
	UPROPERTY(BlueprintReadOnly, Category="Task Status")
	float TimeRemaining = 0;
	/// Whether this task has been started, completed, failed
	UPROPERTY(BlueprintReadOnly, Category="Task Status")
	ESuqsItemStatus bState = ESuqsItemStatus::NotStarted;
};

/// State of a single objective, held by FSuqsQuestStatus
/// Although the completion of an objective could be derived from the completion of all its (mandatory) tasks,
/// we store this separately both as an optimisation, and to support changing tasks on completed quests later
/// without undoing their completion in save games.
USTRUCT(BlueprintType)
struct SUQS_API FSuqsObjectiveState
{
	GENERATED_BODY()
public:
	/// The identifier of the objective this status refers to
	UPROPERTY(BlueprintReadOnly, Category="Objective Status")
	FName Identifier;
	/// Whether this objective has been started, completed, failed (quick access to looking at tasks)
	UPROPERTY(BlueprintReadOnly, Category="Objective Status")
	ESuqsItemStatus bState = ESuqsItemStatus::NotStarted;

	/// List of detailed task status
	UPROPERTY(BlueprintReadOnly, Category="Objective Status")
	TArray<FSuqsTaskState> Tasks;

};

/// State of a single quest, to be held by USuqsStatus
/// Although the completion of a quest could be derived from the completion of all its (mandatory) objectives,
/// we store this separately both as an optimisation, and to support changing quest objectives on completed quests later
/// without undoing their completion in save games.
USTRUCT(BlueprintType)
struct SUQS_API FSuqsQuestState
{
	GENERATED_BODY()
public:
	/// The name of the quest this status refers to
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	FName Name;
	/// Whether this objective has been started, completed, failed (quick access to looking at tasks)
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	ESuqsItemStatus bState = ESuqsItemStatus::NotStarted;

	/// List of detailed objective status, in case you want to see which optional ones were completed
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	TArray<FSuqsObjectiveState> Objectives;
};
