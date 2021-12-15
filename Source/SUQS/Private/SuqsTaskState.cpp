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

	if (IsResolveBlockedOn(ESuqsResolveBarrierCondition::Time))
	{
		ResolveBarrier.TimeRemaining = FMath::Max(ResolveBarrier.TimeRemaining - DeltaTime, 0.f);
		MaybeNotifyParentStatusChange();
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

void USuqsTaskState::SetResolveBarrier(const FSuqsResolveBarrier& Barrier)
{
	ResolveBarrier = Barrier;
	// In case manually changing to free up
	MaybeNotifyParentStatusChange();
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
		default:
			break;
		}

		QueueParentStatusChangeNotification();

	}
}

void USuqsTaskState::QueueParentStatusChangeNotification()
{
	ResolveBarrier = Progression->GetResolveBarrierForTask(TaskDefinition, Status);

	MaybeNotifyParentStatusChange();
	
}

bool USuqsTaskState::IsResolveBlockedOn(ESuqsResolveBarrierCondition Barrier) const
{
	return ResolveBarrier.bPending &&
	   (ResolveBarrier.Conditions & static_cast<uint32>(Barrier)) > 0;
}

void USuqsTaskState::MaybeNotifyParentStatusChange()
{
	// Early-out if barrier has already been processed so we only do this once per status change
	if (!ResolveBarrier.bPending)
		return;

	// Assume cleared
	bool bCleared = true;

	// All conditions have to be fulfilled
	if (IsResolveBlockedOn(ESuqsResolveBarrierCondition::Time))
	{
		if (ResolveBarrier.TimeRemaining > 0)
		{
			bCleared = false;
		}
	}
	if (IsResolveBlockedOn(ESuqsResolveBarrierCondition::Gate))
	{
		if (!Progression->IsGateOpen(ResolveBarrier.Gate))
		{
			bCleared = false;
		}
	}
	if (IsResolveBlockedOn(ESuqsResolveBarrierCondition::Explicit))
	{
		bCleared = ResolveBarrier.bGrantedExplicitly;
	}
	
	if (bCleared)
	{
		switch (Status)
		{
		case ESuqsTaskStatus::NotStarted:
		case ESuqsTaskStatus::InProgress:
		case ESuqsTaskStatus::Failed:
			break;
		case ESuqsTaskStatus::Completed: 
			// Completed serial mandatory tasks are removed
			// Optional tasks, non-serial tasks or failed tasks aren't, since complementary or there to explain failure
			if (IsMandatory() &&
				ParentObjective.IsValid() && ParentObjective->AreTasksSequential() &&
				Progression.IsValid())
				Progression->RaiseTaskRemoved(this);
			break;
			
		default: ;
		}

		ParentObjective->NotifyTaskStatusChanged(this);
		ResolveBarrier.bPending = false;
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

void USuqsTaskState::Resolve()
{
	ResolveBarrier.bGrantedExplicitly = true;
	MaybeNotifyParentStatusChange();
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

bool USuqsTaskState::IsResolveBlocked() const
{
	return !IsIncomplete() &&
		ResolveBarrier.Conditions > 0 &&
		ResolveBarrier.bPending;
}

void USuqsTaskState::NotifyGateOpened(const FName& GateName)
{
	if (IsResolveBlockedOn(ESuqsResolveBarrierCondition::Gate) && ResolveBarrier.Gate == GateName)
		MaybeNotifyParentStatusChange();
}
