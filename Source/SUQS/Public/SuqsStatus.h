#pragma once

#include "CoreMinimal.h"
#include "SuqsQuest.h"
#include "Engine/DataTable.h"
#include "UObject/Object.h"
#include "SuqsStatusStructs.h"
#include "SuqsStatus.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSuqsState, Verbose, Verbose);

/**
 * Holder for all the state relating to quests and their objectives/tasks for a single player.
 * Add this somewhere that's useful to you, e.g. your PlayerState or GameInstance.
 */
UCLASS(BlueprintType)
class SUQS_API USuqsStatus : public UObject
{
	GENERATED_BODY()
public:
	/// Provide one or more data assets which define the quests that this status is tracking against.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest Setup")
	TArray<UDataTable*> QuestDataTables;

protected:
	/// Unified quest defs, combined from all entries in QuestDataTables
	UPROPERTY()
	TMap<FName, FSuqsQuest> QuestDefinitions;
	/// Status of active quests. Quests which have not been activated yet and not completed won't be here
	UPROPERTY(SaveGame)
	TArray<FSuqsQuestStatus> ActiveQuests;
	/// Information about completed quests
	UPROPERTY(SaveGame)
	TArray<FSuqsQuestStatus> CompletedQuests;

public:
	virtual void PostLoad() override;
};
