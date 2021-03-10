#include "SuqsQuestState.h"

#include "SuqsObjectiveState.h"
#include "SuqsTaskState.h"

void USuqsQuestState::Initialise(FSuqsQuest* Def, USuqsPlayState* Root)
{
	// We always build quest state from the master quest definition
	// Then when we restore, we do it into this structure.
	// That means there's never a chance that the state doesn't match the definitions (breaking changes to quests will
	// have to be handled another way)

	// Quest definitions are static data so it's OK to keep this (it's owned by parent)
	QuestDefinition = Def;
	PlayState = Root;
	bStatus = ESuqsQuestStatus::NotStarted;
	FastTaskLookup.Empty();

	for (const auto& ObjDef : Def->Objectives)
	{
		auto Obj = NewObject<USuqsObjectiveState>(this);
		Obj->Initialise(&ObjDef, this, Root);
		Objectives.Add(Obj);

		for (auto Task : Obj->Tasks)
		{
			FastTaskLookup.Add(Task->GetIdentifier(), Task);
		}
	}
	
	
}

void USuqsQuestState::Tick(float DeltaTime)
{
	for (auto& Objective : Objectives)
	{
		Objective->Tick(DeltaTime);
	}
	
}


USuqsTaskState* USuqsQuestState::FindTask(const FName& Identifier) const
{
	return FastTaskLookup.FindChecked(Identifier);
}
