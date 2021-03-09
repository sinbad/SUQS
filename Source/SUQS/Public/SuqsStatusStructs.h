#pragma once
#include "CoreMinimal.h"
#include "SuqsStatusStructs.generated.h"


/// Status of a single task, held by FSuqsObjectiveStatus
USTRUCT(BlueprintType)
struct SUQS_API FSuqsTaskStatus
{
	GENERATED_BODY()
public:
	/// The identifier of the objective this status refers to
	UPROPERTY(BlueprintReadOnly, Category="Task Status")
	FName Identifier;
	/// Current number (vs target number of quest)
	UPROPERTY(BlueprintReadOnly, Category="Task Status")
	int Number;
	/// Current time elapsed, if task has a time limit
	UPROPERTY(BlueprintReadOnly, Category="Task Status")
	float ElapsedTime;
	/// Whether this task has been completed (shorthand to comparing to task definition)
	UPROPERTY(BlueprintReadOnly, Category="Task Status")
	bool bCompleted;
};

/// Status of a single objective, held by FSuqsQuestStatus
/// Although the completion of an objective could be derived from the completion of all its (mandatory) tasks,
/// we store this separately both as an optimisation, and to support changing tasks on completed quests later
/// without undoing their completion in save games.
USTRUCT(BlueprintType)
struct SUQS_API FSuqsObjectiveStatus
{
	GENERATED_BODY()
public:
	/// The identifier of the objective this status refers to
	UPROPERTY(BlueprintReadOnly, Category="Objective Status")
	FName Identifier;
	/// Whether this objective has been completed
	UPROPERTY(BlueprintReadOnly, Category="Objective Status")
	bool bCompleted;

	/// List of detailed task status
	UPROPERTY(BlueprintReadOnly, Category="Objective Status")
	TArray<FSuqsTaskStatus> Tasks;

};

/// Status of a single quest, to be held by USuqsStatus
/// Although the completion of a quest could be derived from the completion of all its (mandatory) objectives,
/// we store this separately both as an optimisation, and to support changing quest objectives on completed quests later
/// without undoing their completion in save games.
USTRUCT(BlueprintType)
struct SUQS_API FSuqsQuestStatus
{
	GENERATED_BODY()
public:
	/// The identifier of the quest this status refers to
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	FName Identifier;
	/// Whether this quest has been completed
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	bool bCompleted;

	/// List of detailed objective status, in case you want to see which optional ones were completed
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	TArray<FSuqsObjectiveStatus> Objectives;
};
