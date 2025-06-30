#include "SuqsTaskState.h"

#include <algorithm>
#include "Suqs.h"
#include "SuqsProgression.h"
#include "SuqsWaypointSubsystem.h"
#include "Kismet/GameplayStatics.h"

void USuqsTaskState::Initialise(const FSuqsTask* TaskDef, USuqsObjectiveState* ObjState, USuqsProgression* Root)
{
	TaskDefinition = TaskDef;
	ParentObjective = ObjState;
	Progression = Root;

	bTitleNeedsFormatting = USuqsProgression::GetTextNeedsFormatting(TaskDef->Title);

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

FText USuqsTaskState::GetTitle() const
{
	if (bTitleNeedsFormatting)
		return GetRootProgression()->FormatTaskText(GetParentObjective()->GetParentQuest()->GetIdentifier(),
													 GetIdentifier(),
													 TaskDefinition->Title);
	else
		return TaskDefinition->Title;
}

void USuqsTaskState::SetTimeRemaining(float T)
{
	const float PrevTime = TimeRemaining;
	// Clamp to 0, but allow higher than taskdef time limit if desired
    TimeRemaining = std::max(0.f, T);
		
	if (IsTimeLimited() && TimeRemaining < PrevTime)
	{
		Progression->RaiseTaskUpdated(this);
		if (TimeRemaining <= 0)
		{
			TimeRemaining = 0;
			if (TaskDefinition->TimeLimitCompleteOnExpiry)
			{
				Complete();
			}
			else
			{
				Fail();
			}
		}
	}	
}

void USuqsTaskState::SetResolveBarrier(const FSuqsResolveBarrier& Barrier)
{
	ResolveBarrier = Barrier;
	// In case manually changing to free up
	MaybeNotifyParentStatusChange();
}

void USuqsTaskState::ChangeStatus(ESuqsTaskStatus NewStatus, bool bIgnoreResolveBarriers)
{
	if (Status != NewStatus)
	{
		Status = NewStatus;

		switch(NewStatus)
		{
		case ESuqsTaskStatus::Completed: 
			Progression->RaiseTaskCompleted(this);
			break;
		case ESuqsTaskStatus::Failed:
			Progression->RaiseTaskFailed(this);
			break;
		default:
			Progression->RaiseTaskUpdated(this);
			break;
		}

		QueueParentStatusChangeNotification(bIgnoreResolveBarriers);

	}
}

void USuqsTaskState::QueueParentStatusChangeNotification(bool bIgnoreBarriers)
{
	if (bIgnoreBarriers)
	{
		ResolveBarrier = FSuqsResolveBarrier();
		ResolveBarrier.bPending = true;
	}
	else
	{
		ResolveBarrier = Progression->GetResolveBarrierForTask(TaskDefinition, Status);
	}

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
		ResolveBarrier.bPending = false;
		ParentObjective->NotifyTaskStatusChanged(this);
	}
}

void USuqsTaskState::Fail(bool bIgnoreResolveBarriers)
{
	ChangeStatus(ESuqsTaskStatus::Failed, bIgnoreResolveBarriers);
}

bool USuqsTaskState::Complete(bool bIgnoreResolveBarriers)
{
	if (Status != ESuqsTaskStatus::Completed)
	{
		// Skip validation when loading
		if (!ParentObjective->GetParentQuest()->IsLoading())
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
					UE_LOG(LogSUQS, Warning, TEXT("Tried to complete mandatory task %s out of order, ignoring"), *GetIdentifier().ToString())
					return false;
				}
			}
		}		
		Number = TaskDefinition->TargetNumber;
		ChangeStatus(ESuqsTaskStatus::Completed, bIgnoreResolveBarriers);
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
	const int PrevNumber = Number;
	// Clamp
	Number = std::min(std::max(0, N), TaskDefinition->TargetNumber);

	if (PrevNumber != Number)
	{
		Progression->RaiseTaskUpdated(this);

		if (Number == TaskDefinition->TargetNumber)
			Complete();
		else
			ChangeStatus(Number > 0 ? ESuqsTaskStatus::InProgress : ESuqsTaskStatus::NotStarted);
		
	}
	
}

int USuqsTaskState::GetNumberOutstanding() const
{
	// Number should be limited already so don't waste time clamping
	return GetTargetNumber() - Number;
}

void USuqsTaskState::Reset()
{
	const bool bRaiseUpdate = Number > 0 || TimeRemaining < TaskDefinition->TimeLimit;
	Number = 0;
	TimeRemaining = TaskDefinition->TimeLimit;
	ChangeStatus(ESuqsTaskStatus::NotStarted);

	// There isn't an event for change status back to not started
	if (bRaiseUpdate)
		Progression->RaiseTaskUpdated(this);

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

bool USuqsTaskState::IsHiddenOnCompleteOrFail() const
{
	return !TaskDefinition->bAlwaysVisible && IsMandatory() &&
		(ParentObjective.IsValid() && ParentObjective->AreTasksSequential());
}

bool USuqsTaskState::IsRelevant() const
{
	return
		IsIncomplete() &&
		!bHidden &&
		GetParentObjective() == GetParentObjective()->GetParentQuest()->GetCurrentObjective() &&
		GetParentObjective()->GetParentQuest()->IsIncomplete();
}

USuqsWaypointComponent* USuqsTaskState::GetWaypoint(bool bOnlyEnabled)
{
	if (IsValid(GetWorld()))
	{
		const auto GI = UGameplayStatics::GetGameInstance(this);
		if (IsValid(GI))
		{
			auto Suqs = GI->GetSubsystem<USuqsWaypointSubsystem>();
			return Suqs->GetWaypoint(GetParentObjective()->GetParentQuest()->GetIdentifier(), GetIdentifier(), bOnlyEnabled);
		}
	}

	return nullptr;
}

TArray<USuqsWaypointComponent*> USuqsTaskState::GetWaypoints(bool bOnlyEnabled)
{
	TArray<USuqsWaypointComponent*> Ret;
	if (IsValid(GetWorld()))
	{
		const auto GI = UGameplayStatics::GetGameInstance(this);
		if (IsValid(GI))
		{
			auto Suqs = GI->GetSubsystem<USuqsWaypointSubsystem>();
			Suqs->GetWaypoints(GetParentObjective()->GetParentQuest()->GetIdentifier(), GetIdentifier(), bOnlyEnabled, Ret);
		}
	}
	return Ret;
}

void USuqsTaskState::FinishLoad()
{
	// Derive hidden from other state, since it's not saved
	bHidden = false;
	if (IsMandatory())
	{
		if (IsHiddenOnCompleteOrFail() && (IsCompleted() || IsFailed()))
		{
			bHidden = true;
		}
		else
		{
			// If incomplete, then this is hidden if objective is set to sequential tasks and this isn't the next one
			auto Obj = GetParentObjective();
			if (Obj->AreTasksSequential() && Obj->GetNextMandatoryTask() != this)
			{
				bHidden = true;
			}
		}
	}

	// Also need to determine if the title needs formatting, since Initialise() is not called
	bTitleNeedsFormatting = USuqsProgression::GetTextNeedsFormatting(TaskDefinition->Title); 
}
