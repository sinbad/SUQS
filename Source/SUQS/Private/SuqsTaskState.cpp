#include "SuqsTaskState.h"

#include <algorithm>
#include "Suqs.h"
#include "SuqsProgression.h"

void USuqsTaskState::Initialise(const FSuqsTask* TaskDef, USuqsObjectiveState* ObjState, USuqsProgression* Root)
{
	TaskDefinition = TaskDef;
	ParentObjective = ObjState;
	Progression = Root;

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
		SetTimeRemaining(TimeRemaining - DeltaTime);
	}	
}


void USuqsTaskState::SetTimeRemaining(float T)
{
	// Clamp to 0, but allow higher than taskdef time limit if desired
    TimeRemaining = std::max(0.f, T);
		
	if (IsTimeLimited())
	{
		Progression->RaiseTaskUpdated(this);
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

		Progression->RaiseTaskUpdated(this);
		switch(NewStatus)
		{
		case ESuqsTaskStatus::Completed: 
			Progression->RaiseTaskCompleted(this);
			break;
		case ESuqsTaskStatus::Failed:
			Progression->RaiseTaskFailed(this);
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
			UE_LOG(LogSUQS, Verbose, TEXT("Tried to complete task %s but parent objective %s is not current, ignoring"),
				*GetIdentifier().ToString(), *ParentObjective->GetIdentifier().ToString())
			return false;
		}
		if (ParentObjective->AreTasksSequential())
		{
			// Only allowed if optional or next in sequence
			if (IsMandatory() && ParentObjective->GetNextMandatoryTask() != this)
			{
				UE_LOG(LogSUQS, Verbose, TEXT("Tried to complete mandatory task %s out of order, ignoring"), *GetIdentifier().ToString())
				return false;
			}
		}
		
		Number = TaskDefinition->TargetNumber;
		ChangeStatus(ESuqsTaskStatus::Completed);
	}
	// Already completed
	return true;
}

int USuqsTaskState::Progress(int Delta)
{
	SetNumber(Number + Delta);

	return GetNumberOutstanding();
}


void USuqsTaskState::SetNumber(int N)
{
	// Clamp
	Number = std::min(std::max(0, N), TaskDefinition->TargetNumber);

	Progression->RaiseTaskUpdated(this);

	if (Number == TaskDefinition->TargetNumber)
		Complete();
	else
		ChangeStatus(Number > 0 ? ESuqsTaskStatus::InProgress : ESuqsTaskStatus::NotStarted);
	
}

int USuqsTaskState::GetNumberOutstanding() const
{
	// Number should be limited already so don't waste time clamping
	return GetTargetNumber() - Number;
}

void USuqsTaskState::Reset()
{
	Number = 0;
	TimeRemaining = TaskDefinition->TimeLimit;
	ChangeStatus(ESuqsTaskStatus::NotStarted);
}
