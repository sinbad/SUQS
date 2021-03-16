#include "SuqsQuestState.h"

#include "SuqsObjectiveState.h"
#include "SuqsProgression.h"
#include "SuqsTaskState.h"

void USuqsQuestState::Initialise(FSuqsQuest* Def, USuqsProgression* Root)
{
	// We always build quest state from the master quest definition
	// Then when we restore, we do it into this structure.
	// That means there's never a chance that the state doesn't match the definitions (breaking changes to quests will
	// have to be handled another way)

	// Quest definitions are static data so it's OK to keep this (it's owned by parent)
	QuestDefinition = Def;
	Progression = Root;
	Status = ESuqsQuestStatus::Incomplete;
	FastTaskLookup.Empty();
	ActiveBranches.Empty();

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
	
	NotifyObjectiveStatusChanged();
}

void USuqsQuestState::Tick(float DeltaTime)
{
	// only tick the current objective
	auto Obj = GetCurrentObjective();
	if (Obj)
	{
		Obj->Tick(DeltaTime);
	}	
}


USuqsTaskState* USuqsQuestState::FindTask(const FName& Identifier) const
{
	return FastTaskLookup.FindChecked(Identifier);
}


void USuqsQuestState::SetBranchActive(FName Branch, bool bActive)
{
	int Changes;
	if (bActive)
		Changes = ActiveBranches.AddUnique(Branch);
	else
		Changes = ActiveBranches.Remove(Branch);

	if (Changes > 0)
		NotifyObjectiveStatusChanged();
}

bool USuqsQuestState::IsBranchActive(FName Branch)
{
	// No branch is always active
	if (Branch.IsNone())
		return true;
	
	return ActiveBranches.Contains(Branch);
}


const FText& USuqsQuestState::GetDescription() const
{
	switch (Status)
	{
	case ESuqsQuestStatus::Incomplete:
	case ESuqsQuestStatus::Failed: 
	case ESuqsQuestStatus::Unavailable: 
	default:
		return QuestDefinition->DescriptionWhenActive;
	case ESuqsQuestStatus::Completed:
		return QuestDefinition->DescriptionWhenCompleted.IsEmpty() ? QuestDefinition->DescriptionWhenActive : QuestDefinition->DescriptionWhenCompleted;
	}
}

USuqsObjectiveState* USuqsQuestState::GetCurrentObjective() const
{
	if (CurrentObjectiveIndex >= 0 && CurrentObjectiveIndex < Objectives.Num())
		return Objectives[CurrentObjectiveIndex];

	return nullptr;
}

USuqsObjectiveState* USuqsQuestState::GetObjective(const FName& Identifier) const
{
	for (auto O : Objectives)
	{
		if (O->GetIdentifier() == Identifier)
			return O;
	}
	return nullptr;
}

bool USuqsQuestState::IsObjectiveCompleted(const FName& Identifier) const
{
	if (auto O = GetObjective(Identifier))
	{
		return O->IsCompleted();
	}
	return false;
}


bool USuqsQuestState::IsObjectiveFailed(const FName& Identifier) const
{
	if (auto O = GetObjective(Identifier))
	{
		return O->IsFailed();
	}
	return false;
}


bool USuqsQuestState::IsTaskCompleted(const FName& TaskID) const
{
	if (auto T = FindTask(TaskID))
	{
		return T->IsCompleted();
	}
	return false;
}

bool USuqsQuestState::IsTaskFailed(const FName& TaskID) const
{
	if (auto T = FindTask(TaskID))
	{
		return T->IsFailed();
	}
	return false;
}

void USuqsQuestState::Reset()
{
	for (auto Obj : Objectives)
	{
		// This will trigger notifications on change
		Obj->Reset();
	}
}

void USuqsQuestState::ResetBranch(FName Branch)
{
	for (auto Obj : Objectives)
	{
		if (Obj->GetBranch() == Branch)
		{
			// This will trigger notifications on change
			Obj->Reset();
		}
	}
	
}

void USuqsQuestState::Fail()
{
	auto Obj = GetCurrentObjective();
	if (Obj)
	{
		Obj->FailOutstandingTasks();
	}
}

void USuqsQuestState::NotifyObjectiveStatusChanged()
{
	// Re-scan the objectives from top to bottom (this allows ANY change to have been made, including backtracking)
	// The next active objective is the next incomplete one in sequence which is on an active branch
	// If there is no next objective, then the quest is complete.
	CurrentObjectiveIndex = -1;
	bool ObjectivesFailed = false;

	for (int i = 0; i < Objectives.Num(); ++i)
	{
		auto Obj = Objectives[i];
		// The first objective that's on an active branch and incomplete is the current objective
		if (IsBranchActive(Obj->GetBranch()) &&
			Obj->IsIncomplete())
		{
			CurrentObjectiveIndex = i;
			// Call this anyway, may not change but that's OK
			ChangeStatus(ESuqsQuestStatus::Incomplete);
			// first incomplete unfiltered objective is the next one 
			break;
		}
		else
		{
			if (Obj->GetStatus() == ESuqsObjectiveStatus::Failed)
				ObjectivesFailed = true;
		}
	}

	// If any unfiltered objectives failed, we lose
	if (ObjectivesFailed)
		ChangeStatus(ESuqsQuestStatus::Failed);
	else if (CurrentObjectiveIndex == -1)
	{
		// No incomplete objectives, and no failures
		ChangeStatus(ESuqsQuestStatus::Completed);
	}
	else
	{
		// Just in case we go backwards (e.g. reset)
		ChangeStatus(ESuqsQuestStatus::Incomplete);
	}
	

}
void USuqsQuestState::ChangeStatus(ESuqsQuestStatus NewStatus)
{
	if (Status != NewStatus)
	{
		Status = NewStatus;

		switch(NewStatus)
		{
		case ESuqsQuestStatus::Completed: 
			Progression->RaiseQuestCompleted(this);
			break;
		case ESuqsObjectiveStatus::Failed:
			Progression->RaiseQuestFailed(this);
			break;
		case ESuqsQuestStatus::Incomplete:
			Progression->RaiseQuestReset(this);
			break;
		default: break;
		}
	}
}
