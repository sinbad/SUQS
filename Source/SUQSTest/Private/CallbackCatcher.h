#pragma once

#include "CoreMinimal.h"


#include "SuqsProgression.h"
#include "UObject/Object.h"
#include "CallbackCatcher.generated.h"

class USuqsQuestState;
class USuqsObjectiveState;
class USuqsTaskState;
UCLASS()
class UCallbackCatcher : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<USuqsQuestState*> AcceptedQuests;
	UPROPERTY()
	TArray<USuqsQuestState*> CompletedQuests;
	UPROPERTY()
	TArray<USuqsQuestState*> FailedQuests;
	UPROPERTY()
	TArray<USuqsTaskState*> UpdatedTasks;
	UPROPERTY()
	TArray<USuqsTaskState*> CompletedTasks;
	UPROPERTY()
	TArray<USuqsTaskState*> FailedTasks;
	UPROPERTY()
	TArray<USuqsObjectiveState*> CompletedObjectives;
	UPROPERTY()
	TArray<USuqsObjectiveState*> FailedObjectives;
	UFUNCTION()
    void OnQuestAccepted(USuqsQuestState* Quest) { AcceptedQuests.Add(Quest); }
	UFUNCTION()
    void OnQuestCompleted(USuqsQuestState* Quest) { CompletedQuests.Add(Quest); }
	UFUNCTION()
    void OnQuestFailed(USuqsQuestState* Quest) { FailedQuests.Add(Quest); }
	UFUNCTION()
    void OnTaskUpdated(USuqsTaskState* Task) { UpdatedTasks.Add(Task); }
	UFUNCTION()
    void OnTaskCompleted(USuqsTaskState* Task) { CompletedTasks.Add(Task); }
	UFUNCTION()
    void OnTaskFailed(USuqsTaskState* Task) { FailedTasks.Add(Task); }
	UFUNCTION()
    void OnObjectiveCompleted(USuqsObjectiveState* Objective) { CompletedObjectives.Add(Objective); }
	UFUNCTION()
    void OnObjectiveFailed(USuqsObjectiveState* Objective) { FailedObjectives.Add(Objective); }

	void Subscribe(USuqsProgression* Progression);
};
