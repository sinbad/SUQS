// 

#pragma once

#include "CoreMinimal.h"
#include "SuqsTaskState.h"
#include "UObject/Object.h"
#include "SuqsProgressView.generated.h"

/// A "view" on the underlying task state. This is primarily used for multiplayer games, where it's
/// simpler to replicate just a view on the state rather than the real SUQS progress objects. 
USTRUCT(BlueprintType)
struct SUQS_API FSuqsTaskStateView
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
	UPROPERTY(BlueprintReadOnly, Category="Task")
	int CompletedNumber;
	
	/// Current time remaining, if task has a time limit
	UPROPERTY(BlueprintReadOnly, Category="Task")
	float TimeRemaining = 0;
	/// Whether this task has been started, completed, failed
	UPROPERTY(BlueprintReadOnly, Category="Task")
	ESuqsTaskStatus Status = ESuqsTaskStatus::NotStarted;
	
	/// Whether we suggest that this task is hidden from the player right now
	/// This is the case for mandatory, sequential, incomplete tasks beyond the first one
	UPROPERTY(BlueprintReadOnly, Category="Task")
	bool bHidden;

	FSuqsTaskStateView();
	void FromUObject(USuqsTaskState* State);

	bool IsModified(const FSuqsTaskStateView& Rhs) const
	{
		// Only compare things that can change at runtime
		return
			Status != Rhs.Status ||
			CompletedNumber != Rhs.CompletedNumber ||
			TimeRemaining != Rhs.TimeRemaining ||
			bHidden != Rhs.bHidden;
	}

	
};

/// A "view" on the underlying state of a quest. This is primarily used for multiplayer games, where it's
/// simpler to replicate just a view on the state rather than the real SUQS progress objects.
/// Notice that there's no list of objectives in this state view, since it's only here to represent
/// the current objective. The text associated with the current objective is here, but otherwise it's
/// just a 2-level structure as opposed to a 3-level one in the real quest structure.
USTRUCT(BlueprintType)
struct SUQS_API FSuqsQuestStateView
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

	UPROPERTY(BlueprintReadOnly, Category="Quest")
	ESuqsQuestStatus Status = ESuqsQuestStatus::Incomplete;	

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
	void FromUObject(USuqsQuestState* State, bool bIncludeCompletedObjectives);
	
	bool IsModified(const FSuqsQuestStateView& Rhs) const
	{
		// Only compare things that can change at runtime
		return
			Status != Rhs.Status ||
			Description.CompareTo(Rhs.Description) != 0 ||
			CurrentObjectiveIdentifier != Rhs.CurrentObjectiveIdentifier ||
			CurrentObjectiveDescription.CompareTo(Rhs.CurrentObjectiveDescription) != 0;
	}
};

/// A "view" on the underlying state of all quest progress. This is primarily used for multiplayer games, where it's
/// simpler to replicate just a view on the state rather than the real SUQS progress objects.
USTRUCT(BlueprintType)
struct SUQS_API FSuqsProgressView
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Progress")
	TArray<FSuqsQuestStateView> ActiveQuests;


	FSuqsProgressView();
	void FromUObject(USuqsProgression* State, bool bIncludeCompletedObjectives);

};

UENUM(BlueprintType)
enum class ESuqsProgressViewDiffCategory : uint8
{
	Quest,
	Task
};

UENUM(BlueprintType)
enum class ESuqsProgressViewDiffChangeType : uint8
{
	Added,
	/// Some details OTHER than completed/failed changed; e.g. description
	Modified,
	Completed,
	Failed,
	Removed
};


USTRUCT(BlueprintType)
struct SUQS_API FSuqsProgressViewDiffEntry
{
	GENERATED_BODY()

	/// What category of change this is (quest, task)
	UPROPERTY(BlueprintReadOnly)
	ESuqsProgressViewDiffCategory Category = ESuqsProgressViewDiffCategory::Quest;

	/// What has changed in that category
	UPROPERTY(BlueprintReadOnly)
	ESuqsProgressViewDiffChangeType ChangeType = ESuqsProgressViewDiffChangeType::Added;

	/// Identifier of the quest (always present). Use USuqsProgressViewHelpers to locate this in the progress view
	UPROPERTY(BlueprintReadOnly)
	FName QuestID = NAME_None;

	/// Identifier of the task, if this difference concerns a task. Use USuqsProgressViewHelpers to locate this in the progress view
	UPROPERTY(BlueprintReadOnly)
	FName TaskID = NAME_None;

};

USTRUCT(BlueprintType)
struct SUQS_API FSuqsProgressViewDiff
{
	GENERATED_BODY()

	/// The list of changes that have occurred
	UPROPERTY(BlueprintReadOnly)
	TArray<FSuqsProgressViewDiffEntry> Entries;
	
};


UCLASS()
class SUQS_API USuqsProgressViewHelpers : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Given a before & after view on progress, generate a differences structure which helps perform
	 * delta updates in a UI rather than re-creating everything.
	 * @param Before The progress view before the changes
	 * @param After The progress view after the changes
	 * @param OutDiff The differences between them
	 * @return Whether there were any differences
	 */
	static bool GetProgressViewDifferences(const FSuqsProgressView& Before, const FSuqsProgressView& After, FSuqsProgressViewDiff& OutDiff);

	/**
	 * Given a before & after view on progress, generate a differences structure which helps perform
	 * delta updates in a UI rather than re-creating everything.
	 * @param Before The progress view before the changes
	 * @param After The progress view after the changes
	 * @param Differences The differences between them
	 * @param bWasDifferent Whether there were any differences
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Progress View Differences", Category="SUQS")
	static void GetProgressViewDifferencesBP(const FSuqsProgressView& Before, const FSuqsProgressView& After, FSuqsProgressViewDiff& Differences, bool& bWasDifferent);

	/**
	 * Helper function to locate quest state in a progress view snapshot.
	 * @param ProgressView The progress view snapshot
	 * @param QuestID The identifier of the quest
	 * @param Quest Output quest state details, if found
	 * @param bWasFound Whether the quest state was found
	 */
	UFUNCTION(BlueprintCallable, Category="SUQS")
	static void GetQuestStateFromProgressView(const FSuqsProgressView& ProgressView, FName QuestID, FSuqsQuestStateView& Quest, bool& bWasFound);

	/**
	 * Helper function to locate task state in a progress view snapshot.
	 * @param ProgressView The progress view snapshot
	 * @param QuestID The identifier of the quest
	 * @param TaskID The identifier of the task
	 * @param Quest Output quest state details, if found
	 * @param Task Output task state details, if found
	 * @param bWasFound Whether the task state was found
	 */
	UFUNCTION(BlueprintCallable, Category="SUQS")
	static void GetTaskStateFromProgressView(const FSuqsProgressView& ProgressView, FName QuestID, FName TaskID, FSuqsQuestStateView& Quest, FSuqsTaskStateView& Task, bool& bWasFound);

};
