// 

#pragma once

#include "CoreMinimal.h"
#include "SuqsTaskState.h"
#include "UObject/Object.h"
#include "SuqsProgressView.generated.h"

/// A "view" on the underlying task state. This is primarily used for multiplayer games, where it's
/// simpler to replicate just a view on the state rather than the real SUQS progress objects. 
USTRUCT(BlueprintType)
struct FSuqsTaskStateView
{
	GENERATED_BODY()

	/// Unique identifier of the task. Must be unique within the *quest* (not just within objective)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Task")
	FName Identifier;

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
	
	/// Current number completed (vs target number)
	UPROPERTY(BlueprintReadOnly, Category="Task State")
	int CompletedNumber;
	
	/// Current time remaining, if task has a time limit
	UPROPERTY(BlueprintReadOnly, Category="Task State")
	float TimeRemaining = 0;
	/// Whether this task has been started, completed, failed
	UPROPERTY(BlueprintReadOnly, Category="Task State")
	ESuqsTaskStatus Status = ESuqsTaskStatus::NotStarted;
	/// Whether we suggest that this task is hidden from the player right now
	/// This is the case for mandatory, sequential, incomplete tasks beyond the first one
	UPROPERTY(BlueprintReadOnly, Category="Task State")
	bool bHidden;

	FSuqsTaskStateView();
	void FromUObject(USuqsTaskState* State);
	
};

/// A "view" on the underlying state of a quest. This is primarily used for multiplayer games, where it's
/// simpler to replicate just a view on the state rather than the real SUQS progress objects.
/// Notice that there's no list of objectives in this state view, since it's only here to represent
/// the current objective. The text associated with the current objective is here, but otherwise it's
/// just a 2-level structure as opposed to a 3-level one in the real quest structure.
USTRUCT(BlueprintType)
struct FSuqsQuestStateView
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	FName Identifier;

	/// Optional list of labels to associate with the quest. Could be used for categorisation etc 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	TArray<FName> Labels;
	
	/// Summary title of the quest
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	FText Title;

	/// Current detailed description of the quest (alters as status changes)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	FText Description;

	/// Identifier of the current objective. This can be helpful to know whether the whole group of tasks
	/// has reset.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	FName CurrentObjectiveIdentifier;
	
	/// Optional description of the current objective, which will change as quest progresses
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	FText CurrentObjectiveDescription;

	/// Current tasks
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Quest")
	TArray<FSuqsTaskStateView> CurrentTasks;

	FSuqsQuestStateView();
	void FromUObject(USuqsQuestState* State);
};

/// A "view" on the underlying state of all quest progress. This is primarily used for multiplayer games, where it's
/// simpler to replicate just a view on the state rather than the real SUQS progress objects.
USTRUCT(BlueprintType)
struct FSuqsProgressView
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Progress")
	TArray<FSuqsQuestStateView> ActiveQuests;


	FSuqsProgressView();
	void FromUObject(USuqsProgression* State);

};