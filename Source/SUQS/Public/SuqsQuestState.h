// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SuqsQuest.h"
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

/**
 * Quest state
 */
UCLASS()
class SUQS_API USuqsQuestState : public UObject
{
	GENERATED_BODY()

	friend class USuqsPlayState;
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


	UPROPERTY()
	TMap<FName, USuqsTaskState*> FastTaskLookup;

	// Pointer to quest definition, for convenience (this is static data)
	FSuqsQuest* QuestDefinition;
	TWeakObjectPtr<USuqsPlayState> PlayState;

	void Initialise(FSuqsQuest* Def, USuqsPlayState* Root);
	void Tick(float DeltaTime);
	void ChangeStatus(ESuqsQuestStatus NewStatus);
	
public:
	ESuqsQuestStatus GetStatus() const { return Status; }
	const TArray<USuqsObjectiveState*>& GetObjectives() const { return Objectives; }
	const TArray<FName>& GetActiveBranches() const { return ActiveBranches; }

	/// Get the unique quest identifier
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FName& GetIdentifier() const { return QuestDefinition->Identifier; }
	/// Get the quest title
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FText& GetTitle() const { return QuestDefinition->Title; }
	/// Get the current description for this quest (just the top-level description)
	/// For any additional objective description, see GetCurrentObjective()->GetDescription();
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FText& GetDescription() const;
	
	/// Set an objective branch to be active in this quest. Objectives associated with this branch will then be allowed
	/// to activate.
	UFUNCTION(BlueprintCallable)
	void SetBranchActive(FName Branch, bool bActive);

	/// Return whether an objective branch is active or not
	UFUNCTION(BlueprintCallable)
    bool IsBranchActive(FName Branch);

	/// Get the current objective on this quest. Will return null if quest is complete.
	UFUNCTION(BlueprintCallable, BlueprintPure)
	USuqsObjectiveState* GetCurrentObjective() const;

	/// Reset all the progress on this quest back to the initial state
	UFUNCTION(BlueprintCallable)
	void Reset();

	/// Reset all the progress on objectives associated with the named quest branch
	UFUNCTION(BlueprintCallable)
    void ResetBranch(FName Branch);

	void NotifyObjectiveStatusChanged();

	/// Find a task with the given identifier in this quest
	USuqsTaskState* FindTask(const FName& Identifier) const;

};
