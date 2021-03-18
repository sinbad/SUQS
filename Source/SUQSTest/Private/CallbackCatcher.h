#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CallbackCatcher.generated.h"

class USuqsQuestState;
UCLASS()
class UCallbackCatcher : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<USuqsQuestState*> AcceptedQuests;
	TArray<USuqsQuestState*> CompletedQuests;
	TArray<USuqsQuestState*> FailedQuests;
	UFUNCTION()
    void OnQuestAccepted(USuqsQuestState* Quest) { AcceptedQuests.Add(Quest); }
	UFUNCTION()
    void OnQuestCompleted(USuqsQuestState* Quest) { CompletedQuests.Add(Quest); }
	UFUNCTION()
    void OnQuestFailed(USuqsQuestState* Quest) { FailedQuests.Add(Quest); }
};
