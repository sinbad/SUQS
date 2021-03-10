#include "SuqsObjectiveState.h"
#include "SuqsPlayState.h"
#include "SuqsTaskState.h"

void USuqsObjectiveState::Initialise(const FSuqsObjective* ObjDef, USuqsQuestState* QuestState,
	USuqsPlayState* Root)
{
	ObjectiveDefinition = ObjDef;
	ParentQuest = QuestState;
	PlayState = Root;

	bStatus = ESuqsObjectiveStatus::NotStarted;

	for (const auto& TaskDef : ObjDef->Tasks)
	{
		auto Task = NewObject<USuqsTaskState>(this);
		Task->Initialise(&TaskDef, this, Root);
		Tasks.Add(Task);
	}	
	
}

void USuqsObjectiveState::Tick(float DeltaTime)
{
	for (auto& Task : Tasks)
	{
		Task->Tick(DeltaTime);
	}
	
}
