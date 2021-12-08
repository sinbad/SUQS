#pragma once

#include "CoreMinimal.h"
#include "SuqsSaveData.generated.h"

struct FSuqsResolveBarrier;
/// Saved state of any progression barrier
USTRUCT(BlueprintType)
struct SUQS_API FSuqsResolveBarrierStateData
{
	GENERATED_BODY()

public:
	int32 Conditions;
	float TimeRemaining;
	FString Gate;
	bool bGrantedExplicitly;
	bool bPending;
	::FSuqsResolveBarrierStateData& operator=(const FSuqsResolveBarrier& Barrier);
	void SaveToArchive(FArchive& Ar);
	void LoadFromArchive(FArchive& Ar, int FileVersion);
};
/** Saved state data for a task. Objectives are implied from this.
*/
USTRUCT(BlueprintType)
struct SUQS_API FSuqsTaskStateData
{
	GENERATED_BODY()

public:
	FString Identifier;
	int Number;
	float TimeRemaining;
	FSuqsResolveBarrierStateData ResolveBarrier;

	// We don't store status or hidden, those are derived from everything else
	// Although we do store quest status to allow archived quests to stay archived if changed.

	void SaveToArchive(FArchive& Ar);
	void LoadFromArchive(FArchive& Ar, int FileVersion);
	
};


UENUM(BlueprintType)
enum class ESuqsQuestDataStatus : uint8
{
	Incomplete = 0,
	Completed = 2,
	Failed = 4
};

/** Saved state data for a quest.
 */
USTRUCT(BlueprintType)
struct SUQS_API FSuqsQuestStateData
{
	GENERATED_BODY()

public:
	FString Identifier;
	// We do store the overall quest state, to avoid resurrecting quests if they're given new tasks in an update but save game has them complete
	ESuqsQuestDataStatus Status;
	TArray<FSuqsTaskStateData> TaskData;
	TArray<FString> ActiveBranches;
	FSuqsResolveBarrierStateData ResolveBarrier;

	void SaveToArchive(FArchive& Ar);
	void LoadFromArchive(FArchive& Ar, int FileVersion);

};


/**
 * This represents the save data as persistent info on disk.
 * We use a tree of structs as an intermediary to allow user code to perform fixups on save data
 * before it's actually loaded into the quest state if needed.
 * We ONLY store quests and tasks, not objectives. Objective data is implied from the quest definitions
 * so we only need to store the state of tasks (and quests as a top level since tasks IDs are only unique within quests)
 */
USTRUCT(BlueprintType)
struct SUQS_API FSuqsSaveData
{
	GENERATED_BODY()

public:

	int Version;
	TArray<FSuqsQuestStateData> QuestData;
	TArray<FString> GlobalActiveBranches;
	TArray<FString> OpenGates;

	void SaveToArchive(FArchive& Ar);
	void LoadFromArchive(FArchive& Ar);

	void Serialize(FArchive& Ar);
};
