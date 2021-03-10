#include "SuqsQuest.h"

const FSuqsTask* FSuqsObjective::FindTask(const FName& Id) const
{
	for (auto& Task : Tasks)
	{
		if (Task.Identifier == Id)
			return &Task;
	}
	return nullptr;
	
}

const FSuqsObjective* FSuqsQuest::FindObjective(const FName& Id) const
{
	for (auto& Obj : Objectives)
	{
		if (Obj.Identifier == Id)
			return &Obj;
	}
	return nullptr;
}
