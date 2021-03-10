#include "SuqsTaskState.h"

void USuqsTaskState::Initialise(const FSuqsTask* TaskDef, USuqsObjectiveState* ObjState, USuqsPlayState* Root)
{
	TaskDefinition = TaskDef;
	ParentObjective = ObjState;
	PlayState = Root;
}

void USuqsTaskState::Tick(float DeltaTime)
{
	if (bTimeLimit && TimeRemaining > 0)
	{
		TimeRemaining -= DeltaTime;
		if (TimeRemaining <= 0)
		{
			TimeRemaining = 0;
			Fail();
		}
	}
	
}

void USuqsTaskState::Fail()
{
	// TODO
}
