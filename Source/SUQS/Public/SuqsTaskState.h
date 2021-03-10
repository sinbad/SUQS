#pragma once

#include "CoreMinimal.h"

#include "SuqsObjectiveState.h"
#include "UObject/Object.h"
#include "SuqsTaskState.generated.h"

UENUM(BlueprintType)
enum class ESuqsTaskStatus : uint8
{
    /// No progress has been made
    NotStarted = 0,
    /// At least one element of progress has been made
    InProgress = 4,
    /// All mandatory elements have been completed
    Completed = 8,
    /// This item has been failed and cannot be progressed without being explicitly reset
    Failed = 20
};


/**
 * Record of the state of a task in a quest objective
 */
UCLASS()
class SUQS_API USuqsTaskState : public UObject
{
	GENERATED_BODY()

	friend class USuqsObjectiveState;
protected:
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
	ESuqsTaskStatus bStatus = ESuqsTaskStatus::NotStarted;

	const FSuqsTask* TaskDefinition;
	TWeakObjectPtr<USuqsObjectiveState> ParentObjective;
	TWeakObjectPtr<USuqsPlayState> PlayState;

	
	void Initialise(const FSuqsTask* TaskDef, USuqsObjectiveState* ObjState, USuqsPlayState* Root);
	void Tick(float DeltaTime);
public:
	const FName& GetIdentifier() const { return TaskDefinition->Identifier; }
	int GetNumber() const { return Number; }
	bool GetTimeLimit() const { return bTimeLimit; }
	float GetTimeRemaining() const { return TimeRemaining; }
	ESuqsTaskStatus GetStatus() const {  return bStatus; }

	void Fail();

};
