#include "SuqsObjectiveState.h"
#include "SuqsProgression.h"
#include "SuqsTaskState.h"

void USuqsObjectiveState::Initialise(const FSuqsObjective* ObjDef, USuqsQuestState* QuestState,
	USuqsProgression* Root)
{
	ObjectiveDefinition = ObjDef;
	ParentQuest = QuestState;
	Progression = Root;

	Status = ESuqsObjectiveStatus::NotStarted;
	MandatoryTasksNeededToComplete = AreAllMandatoryTasksRequired() ? 0 : ObjDef->NumberOfMandatoryTasksRequired;

	for (const auto& TaskDef : ObjDef->Tasks)
	{
		auto Task = NewObject<USuqsTaskState>(this);
		Task->Initialise(&TaskDef, this, Root);
		Tasks.Add(Task);

		if (AreAllMandatoryTasksRequired() && TaskDef.bMandatory)
			++MandatoryTasksNeededToComplete;
	}

	NotifyTaskStatusChanged();
	
}

void USuqsObjectiveState::Tick(float DeltaTime)
{
	for (auto& Task : Tasks)
	{
		Task->Tick(DeltaTime);
	}

}


const FText& USuqsObjectiveState::GetDescription() const
{
	switch (Status)
	{
	case ESuqsObjectiveStatus::NotStarted:
	case ESuqsObjectiveStatus::InProgress:
	case ESuqsObjectiveStatus::Failed:
	default:
		return ObjectiveDefinition->DescriptionWhenActive;
	case ESuqsObjectiveStatus::Completed:
		// Don't replace with active def when blank, blank is OK
		return ObjectiveDefinition->DescriptionWhenCompleted;
	}
}

void USuqsObjectiveState::Reset()
{
	for (auto Task : Tasks)
	{
		// This will cause notifications
		Task->Reset();
	}
}

void USuqsObjectiveState::FailOutstandingTasks()
{
	for (auto Task : Tasks)
	{
		if (Task->IsIncomplete() && !Task->bHidden)
		{
			Task->Fail();
		}
	}
}

void USuqsObjectiveState::CompleteAllMandatoryTasks()
{
	for (auto Task : Tasks)
	{
		if (Task->IsMandatory())
		{
			// Override failed as well
			Task->Complete();
			if (!AreAllMandatoryTasksRequired())
				break;
		}
	}	
}

USuqsTaskState* USuqsObjectiveState::GetNextMandatoryTask() const
{
	for (auto Task : Tasks)
	{
		if (Task->IsIncomplete() && Task->IsMandatory())
		{
			return Task;
		}
	}
	return nullptr;
}

void USuqsObjectiveState::GetAllRelevantTasks(TArray<USuqsTaskState*>& RelevantTasksOut) const
{
	RelevantTasksOut.Empty();
	bool bAddedMandatoryTask = false;
	for (auto Task : Tasks)
	{
		// Completed / failed tasks
		if (!Task->IsIncomplete())
		{
			RelevantTasksOut.Add(Task);
		}
		else
		{
			if (Task->IsMandatory())
			{
				// Only add either the first incomplete mandatory task, or all of them if non-sequential
				if (!bAddedMandatoryTask || !AreTasksSequential())
				{
					RelevantTasksOut.Add(Task);
					bAddedMandatoryTask = true;
				}
			}
			else
			{
				// Optional tasks are always added
				RelevantTasksOut.Add(Task);
			}
		}
	}
}


void USuqsObjectiveState::GetIncompleteTasks(TArray<USuqsTaskState*>& IncompleteTasksOut) const
{
	IncompleteTasksOut.Empty();
	for (auto Task : Tasks)
	{
		if (Task->IsIncomplete())
			IncompleteTasksOut.Add(Task);
	}
}

void USuqsObjectiveState::GetCompletedTasks(TArray<USuqsTaskState*>& CompletedTasksOut) const
{
	CompletedTasksOut.Empty();
	for (auto Task : Tasks)
	{
		if (Task->GetStatus() == ESuqsTaskStatus::Completed)
			CompletedTasksOut.Add(Task);
	}
}

void USuqsObjectiveState::GetFailedTasks(TArray<USuqsTaskState*>& FailedTasksOut) const
{
	FailedTasksOut.Empty();
	for (auto Task : Tasks)
	{
		if (Task->GetStatus() == ESuqsTaskStatus::Failed)
			FailedTasksOut.Add(Task);
	}
}

bool USuqsObjectiveState::IsOnActiveBranch() const
{
	return ParentQuest->IsBranchActive(GetBranch());
}

void USuqsObjectiveState::NotifyTaskStatusChanged()
{
	// Re-scan our tasks and decide what this means for our own state
	int MandatoryTasksFailed = 0;
	int MandatoryTasksComplete = 0;
	int IncompleteMandatoryTasks = 0;
	for (auto& Task : Tasks)
	{
		Task->bHidden = false;
		if (Task->IsMandatory())
		{
			switch(Task->Status)
			{
			case ESuqsTaskStatus::Completed:
				++MandatoryTasksComplete;
				break;
			case ESuqsTaskStatus::Failed:
				++MandatoryTasksFailed;
				break;
			case ESuqsTaskStatus::NotStarted:
			case ESuqsTaskStatus::InProgress:
				if (ObjectiveDefinition->bSequentialTasks && IncompleteMandatoryTasks > 0)
				{
					// If tasks are sequential, and this is an incomplete task after the first, suggest hiding it
					Task->bHidden = true;
				}
				++IncompleteMandatoryTasks;
				break;
			default:
				break;
			}
		}
	}
	
	if (MandatoryTasksFailed > 0 &&
		(IncompleteMandatoryTasks + MandatoryTasksComplete) < MandatoryTasksNeededToComplete)
	{
		ChangeStatus(ESuqsObjectiveStatus::Failed);
	}
	else if (MandatoryTasksComplete >= MandatoryTasksNeededToComplete)
	{
		ChangeStatus(ESuqsObjectiveStatus::Completed);
	}
	else
	{
		ChangeStatus(MandatoryTasksComplete > 0 ? ESuqsObjectiveStatus::InProgress : ESuqsObjectiveStatus::NotStarted);
	}

}

void USuqsObjectiveState::NotifyGateOpened(const FName& GateName)
{
	// This one proceeds downards to children
	for (auto Task : Tasks)
	{
		Task->NotifyGateOpened(GateName);
	}
	
}

void USuqsObjectiveState::ChangeStatus(ESuqsObjectiveStatus NewStatus)
{
	if (Status != NewStatus)
	{
		Status = NewStatus;

		switch(NewStatus)
		{
		case ESuqsObjectiveStatus::Completed: 
			Progression->RaiseObjectiveCompleted(this);
			break;
		case ESuqsObjectiveStatus::Failed:
			Progression->RaiseObjectiveFailed(this);
			break;
		default: break;
		}

		// No barriers on objectives, just tasks and quests
		// Objectives are just groupings and inherit their behaviour from task
		ParentQuest->NotifyObjectiveStatusChanged();
		
	}
}
