#include "SuqsTaskState.h"

#include <algorithm>
#include "SuqsModule.h"
#include "SuqsPlayState.h"

void USuqsTaskState::Initialise(const FSuqsTask* TaskDef, USuqsObjectiveState* ObjState, USuqsPlayState* Root)
{
	TaskDefinition = TaskDef;
	ParentObjective = ObjState;
	PlayState = Root;

	Reset();
}

void USuqsTaskState::Tick(float DeltaTime)
{
	// Don't reduce time when task is hidden (e.g. not the next in sequence)
	// Also not when also completed / failed
	if (!bHidden &&
		IsIncomplete() && 
		IsTimeLimited() &&
		TimeRemaining > 0)
	{
		TimeRemaining -= DeltaTime;
		
		PlayState->RaiseTaskUpdated(this);
		if (TimeRemaining <= 0)
		{
			TimeRemaining = 0;
			Fail();
		}
	}
	
}

void USuqsTaskState::ChangeStatus(ESuqsTaskStatus NewStatus)
{
	if (Status != NewStatus)
	{
		Status = NewStatus;

		PlayState->RaiseTaskUpdated(this);
		switch(NewStatus)
		{
		case ESuqsTaskStatus::Completed: 
			PlayState->RaiseTaskCompleted(this);
			break;
		case ESuqsTaskStatus::Failed:
			PlayState->RaiseTaskFailed(this);
			break;
		default: break;
		}

		ParentObjective->NotifyTaskStatusChanged();		
	}
}


void USuqsTaskState::Fail()
{
	ChangeStatus(ESuqsTaskStatus::Failed);
}

bool USuqsTaskState::Complete()
{
	if (Status != ESuqsTaskStatus::Completed)
	{
		// Check sequencing
		if (ParentObjective->GetParentQuest()->GetCurrentObjective() != ParentObjective)
		{
			UE_LOG(LogSuqs, Warning, TEXT("Tried to complete task %s but parent objective %s is not current, ignoring"),
				*GetIdentifier().ToString(), *ParentObjective->GetIdentifier().ToString())
			return false;
		}
		if (ParentObjective->AreTasksSequential())
		{
			// Only allowed if optional or next in sequence
			if (IsMandatory() && ParentObjective->GetNextMandatoryTask() != this)
			{
				UE_LOG(LogSuqs, Warning, TEXT("Tried to complete mandatory task %s out of order, ignoring"), *GetIdentifier().ToString())
				return false;
			}
		}
		
		Number = TaskDefinition->TargetNumber;
		ChangeStatus(ESuqsTaskStatus::Completed);
	}
	return true;
}

void USuqsTaskState::Progress(int Delta)
{
	Number += Delta;
	Number = std::min(std::max(0, Number), TaskDefinition->TargetNumber);

	PlayState->RaiseTaskUpdated(this);

	if (Number == TaskDefinition->TargetNumber)
		Complete();
}

void USuqsTaskState::Reset()
{
	Number = 0;
	TimeRemaining = TaskDefinition->TimeLimit;
	ChangeStatus(ESuqsTaskStatus::NotStarted);
}
