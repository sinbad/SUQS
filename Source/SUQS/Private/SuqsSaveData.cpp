#include "SuqsSaveData.h"

#include "SuqsQuestState.h"

constexpr int CurrentFileVersion = 2;

constexpr int FileVersion_AddedOpenGates = 2;
constexpr int FileVersion_AddedBarrierState = 2;


void FSuqsProgressionBarrierStateData::SaveToArchive(FArchive& Ar)
{
	Ar << Conditions;
	Ar << TimeRemaining;
	Ar << Gate;
	Ar << bPending;
}

void FSuqsProgressionBarrierStateData::LoadFromArchive(FArchive& Ar, int FileVersion)
{
	Ar << Conditions;
	Ar << TimeRemaining;
	Ar << Gate;
	Ar << bPending;
}

FSuqsProgressionBarrierStateData& FSuqsProgressionBarrierStateData::operator=(
	const FSuqsProgressionBarrier& PB)
{
	Conditions = PB.Conditions;
	TimeRemaining = PB.TimeRemaining;
	Gate = PB.Gate.IsNone() ? "" : PB.Gate.ToString();
	bPending = PB.bPending;
	return *this;
}

void FSuqsTaskStateData::SaveToArchive(FArchive& Ar)
{
	Ar << Identifier;
	Ar << Number;
	Ar << TimeRemaining;
	ProgressionBarrier.SaveToArchive(Ar);
	
}

void FSuqsTaskStateData::LoadFromArchive(FArchive& Ar, int FileVersion)
{
	if (FileVersion != CurrentFileVersion)
	{
		// Do any conversion here
	}

	Ar << Identifier;
	Ar << Number;
	Ar << TimeRemaining;

	if (FileVersion >= FileVersion_AddedBarrierState)
	{
		ProgressionBarrier.LoadFromArchive(Ar, FileVersion);
	}
	
}

void FSuqsQuestStateData::SaveToArchive(FArchive& Ar)
{
	Ar << Identifier;
	uint8 IntStatus = static_cast<uint8>(Status);
	Ar << IntStatus;

	Ar << ActiveBranches;

	ProgressionBarrier.SaveToArchive(Ar);

	int NumTasks = TaskData.Num();
	Ar << NumTasks;

	for (auto& T : TaskData)
	{
		T.SaveToArchive(Ar);
	}
	
}

void FSuqsQuestStateData::LoadFromArchive(FArchive& Ar, int FileVersion)
{
	if (FileVersion != CurrentFileVersion)
	{
		// Do any conversion here
	}
	
	Ar << Identifier;
	uint8 IntStatus;
	Ar << IntStatus;
	Status = static_cast<ESuqsQuestDataStatus>(IntStatus);

	Ar << ActiveBranches;

	if (FileVersion >= FileVersion_AddedBarrierState)
	{
		ProgressionBarrier.LoadFromArchive(Ar, FileVersion);
	}

	int NumTasks;
	Ar << NumTasks;

	TaskData.SetNum(NumTasks);
	for (int i = 0; i < NumTasks; ++i)
	{
		TaskData[i].LoadFromArchive(Ar, FileVersion);
	}
	
}

void FSuqsSaveData::SaveToArchive(FArchive& Ar)
{
	// Version
	int V = CurrentFileVersion;
	Ar << V;

	// Global branches
	Ar << GlobalActiveBranches;
	// Open gates
	Ar << OpenGates;
	
	// Quests
	int NumQuests = QuestData.Num();
	Ar << NumQuests;
	for (auto& Q : QuestData)
	{
		Q.SaveToArchive(Ar);
	}
}

void FSuqsSaveData::LoadFromArchive(FArchive& Ar)
{
	// Version
	int FileVersion = 0;
	Ar << FileVersion;

	if (FileVersion != CurrentFileVersion)
	{
		// Here's where you do any fixes for previous versions		
	}

	// Global branches
	Ar << GlobalActiveBranches;
	if (FileVersion >= FileVersion_AddedOpenGates)
	{
		Ar << OpenGates;
	}
	else
	{
		OpenGates.Empty();
	}

	// Active & archived quests go together
	int NumQuests = QuestData.Num();
	Ar << NumQuests;
	QuestData.SetNum(NumQuests);
	for (int i = 0; i < NumQuests; ++i)
	{
		QuestData[i].LoadFromArchive(Ar, FileVersion);
	}
	
}

void FSuqsSaveData::Serialize(FArchive& Ar)
{
	if (Ar.IsLoading())
		LoadFromArchive(Ar);
	else
		SaveToArchive(Ar);
}
