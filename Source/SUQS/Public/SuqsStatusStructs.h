#pragma once
#include "CoreMinimal.h"
#include "SuqsStatusStructs.generated.h"


/// Summarises the state of a quest / objective / task
/// Note that there isn't an "Inactive" entry. Quests which are not available yet
/// just won't be included at all in quest state
UENUM(BlueprintType)
enum class ESuqsSummaryState : uint8
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
    Failed = 20,
	/// This item (objective/task) is hidden. It is essentially ignored even if mandatory. Can be used
	/// to have "dynamic" objectives / tasks
	Hidden = 40,
};

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
	/// Whether this task has been started, completed, failed
	UPROPERTY(BlueprintReadOnly, Category="Task Status")
	ESuqsSummaryState bState = ESuqsSummaryState::NotStarted;
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
	/// Whether this objective has been started, completed, failed (quick access to looking at tasks)
	UPROPERTY(BlueprintReadOnly, Category="Objective Status")
	ESuqsSummaryState bState = ESuqsSummaryState::NotStarted;

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
	/// The name of the quest this status refers to
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	FName Name;
	/// Whether this objective has been started, completed, failed (quick access to looking at tasks)
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	ESuqsSummaryState bState = ESuqsSummaryState::NotStarted;

	/// List of detailed objective status, in case you want to see which optional ones were completed
	UPROPERTY(BlueprintReadOnly, Category="Quest Status")
	TArray<FSuqsObjectiveStatus> Objectives;
};
