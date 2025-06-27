#include "SuqsQuestState.h"

#include "SuqsObjectiveState.h"
#include "SuqsProgression.h"
#include "SuqsTaskState.h"

//PRAGMA_DISABLE_OPTIMIZATION

void USuqsQuestState::Initialise(const FSuqsQuest* Def, USuqsProgression* Root)
{
	// We always build quest state from the master quest definition
	// Then when we restore, we do it into this structure.
	// That means there's never a chance that the state doesn't match the definitions (breaking changes to quests will
	// have to be handled another way)

	// Quest definitions are static data so it's OK to keep this (it's owned by parent)
	QuestDefinition = Def;
	Progression = Root;
	Status = ESuqsQuestStatus::Incomplete;
	ResolveBarrier = Progression->GetResolveBarrierForQuest(QuestDefinition, Status);
	FastTaskLookup.Empty();
	ActiveBranches.Empty();

	bTitleNeedsFormatting = USuqsProgression::GetTextNeedsFormatting(QuestDefinition->Title);
	bActiveDescriptionNeedsFormatting = USuqsProgression::GetTextNeedsFormatting(QuestDefinition->DescriptionWhenActive);
	bCompletedDescriptionNeedsFormatting = USuqsProgression::GetTextNeedsFormatting(QuestDefinition->DescriptionWhenCompleted);

	for (const auto& ObjDef : Def->Objectives)
	{
		auto Obj = NewObject<USuqsObjectiveState>(GetOuter());
		Obj->Initialise(&ObjDef, this, Root);
		Objectives.Add(Obj);

		for (auto Task : Obj->Tasks)
		{
			FastTaskLookup.Add(Task->GetIdentifier(), Task);
		}
	}
	ResetBranches();
	
	NotifyObjectiveStatusChanged();
}

void USuqsQuestState::Tick(float DeltaTime)
{
	// We tick our own conditions FIRST, otherwise ticking children could change our status and tick us simultaneously
	if (IsResolveBlockedOn(ESuqsResolveBarrierCondition::Time))
	{
		ResolveBarrier.TimeRemaining = FMath::Max(ResolveBarrier.TimeRemaining - DeltaTime, 0.f);
	}
	
	// only tick the current objective
	auto Obj = GetCurrentObjective();
	if (Obj)
	{
		Obj->Tick(DeltaTime);
	}

	MaybeNotifyStatusChange();
	
}


USuqsTaskState* USuqsQuestState::GetTask(const FName& Identifier) const
{
	const auto ppT = FastTaskLookup.Find(Identifier);
	if (ppT)
		return *ppT;
	return nullptr;
}


void USuqsQuestState::SetBranchActive(FName Branch, bool bActive)
{
	if (Branch.IsNone())
		return;

	bool bChanged = false;
	if (bActive)
	{
		if (!ActiveBranches.Contains(Branch))
		{
			ActiveBranches.Add(Branch);
			bChanged = true;
		}
	}
	else
		bChanged = ActiveBranches.Remove(Branch) > 0;

	if (bChanged)
		NotifyObjectiveStatusChanged();
}

void USuqsQuestState::ResetBranches()
{
	if (ActiveBranches != QuestDefinition->DefaultActiveBranches)
	{
		ActiveBranches = QuestDefinition->DefaultActiveBranches;
		NotifyObjectiveStatusChanged();
	}
}

bool USuqsQuestState::IsBranchActive(FName Branch)
{
	// No branch is always active
	if (Branch.IsNone())
		return true;
	
	return ActiveBranches.Contains(Branch);
}

bool USuqsQuestState::CompleteTask(FName TaskID)
{
	if (auto T = GetTask(TaskID))
	{
		return T->Complete();
	}
	return false;
}

void USuqsQuestState::ResolveTask(FName TaskID)
{
	if (auto T = GetTask(TaskID))
	{
		T->Resolve();
	}
}

void USuqsQuestState::FailTask(const FName& TaskID)
{
	if (auto T = GetTask(TaskID))
	{
		T->Fail();
	}	
}

int USuqsQuestState::ProgressTask(FName TaskID, int Delta)
{
	if (auto T = GetTask(TaskID))
	{
		return T->Progress(Delta);
	}
	return 0;
}

void USuqsQuestState::SetTaskNumberCompleted(FName TaskID, int Number)
{
	if (auto T = GetTask(TaskID))
	{
		return T->SetNumber(Number);
	}
}

FText USuqsQuestState::GetTitle() const
{
	if (bTitleNeedsFormatting)
		return GetRootProgression()->FormatQuestText(GetIdentifier(), QuestDefinition->Title);
	else
		return QuestDefinition->Title;
}

const TArray<FName>& USuqsQuestState::GetLabels() const
{
	return QuestDefinition->Labels;
}

bool USuqsQuestState::IsPlayerVisible() const
{
	return QuestDefinition->bPlayerVisible;
}

FText USuqsQuestState::GetDescription() const
{
	const bool bUseCompleted = (Status == ESuqsQuestStatus::Completed && !QuestDefinition->DescriptionWhenCompleted.IsEmpty());

	if (bUseCompleted)
		return bCompletedDescriptionNeedsFormatting
			       ? GetRootProgression()->FormatQuestText(GetIdentifier(),
			                                                      QuestDefinition->DescriptionWhenCompleted)
			       : QuestDefinition->DescriptionWhenCompleted;
	else
		return bActiveDescriptionNeedsFormatting
			       ? GetRootProgression()->FormatQuestText(GetIdentifier(),
			                                                      QuestDefinition->DescriptionWhenActive)
			       : QuestDefinition->DescriptionWhenActive;
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

void USuqsQuestState::GetActiveObjectives(TArray<USuqsObjectiveState*>& ActiveObjectivesOut) const
{
	ActiveObjectivesOut.Reset();
	for (auto O : Objectives)
	{
		if (O->IsOnActiveBranch())
			ActiveObjectivesOut.Add(O);
	}	
}

bool USuqsQuestState::IsResolveBlocked() const
{
	return (IsCompleted() || IsFailed()) &&
		ResolveBarrier.Conditions > 0 &&
		ResolveBarrier.bPending;
}

bool USuqsQuestState::IsObjectiveIncomplete(const FName& Identifier) const
{
	if (auto O = GetObjective(Identifier))
	{
		return O->IsIncomplete();
	}
	return false;
	
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

void USuqsQuestState::ResetObjective(FName Identifier)
{
	if (auto O = GetObjective(Identifier))
	{
		O->Reset();
	}
}

USuqsTaskState* USuqsQuestState::GetNextMandatoryTask() const
{
	if (auto O = GetCurrentObjective())
	{
		return O->GetNextMandatoryTask();
	}
	return nullptr;
}


bool USuqsQuestState::IsTaskIncomplete(const FName& TaskID) const
{
	if (auto T = GetTask(TaskID))
	{
		return T->IsIncomplete();
	}
	return false;
}

bool USuqsQuestState::IsTaskCompleted(const FName& TaskID) const
{
	if (auto T = GetTask(TaskID))
	{
		return T->IsCompleted();
	}
	return false;
}

bool USuqsQuestState::IsTaskFailed(const FName& TaskID) const
{
	if (auto T = GetTask(TaskID))
	{
		return T->IsFailed();
	}
	return false;
}

bool USuqsQuestState::IsTaskRelevant(const FName& TaskID) const
{
	if (auto T = GetTask(TaskID))
	{
		return T->IsRelevant();
	}
	return false;
	
}

void USuqsQuestState::ResetTask(FName TaskID)
{
	if (auto T = GetTask(TaskID))
	{
		T->Reset();
	}	
}

void USuqsQuestState::Reset()
{
	// Let's not raise objective changed events until we're done
	bSuppressObjectiveChangeEvent = true;
	for (auto Obj : Objectives)
	{
		// This will trigger notifications on change
		Obj->Reset();
	}

	ResetBranches();
	bSuppressObjectiveChangeEvent = false;

	// Raise final objective changed here, to better indicate final reset when state is stable
	Progression->RaiseCurrentObjectiveChanged(this);
	
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

void USuqsQuestState::Complete()
{
	for (auto Obj : Objectives)
	{
		Obj->CompleteAllMandatoryTasks();
	}
}

void USuqsQuestState::Resolve()
{
	ResolveBarrier.bGrantedExplicitly = true;
	
	MaybeNotifyStatusChange();
}

void USuqsQuestState::SetResolveBarrier(const FSuqsResolveBarrierStateData& Barrier)
{
	ResolveBarrier = Barrier;
	// In case this completes
	MaybeNotifyStatusChange();
}

void USuqsQuestState::StartLoad()
{
	bIsLoading = true;
}

void USuqsQuestState::FinishLoad()
{
	for (auto O : Objectives)
	{
		O->FinishLoad();
	}
	NotifyObjectiveStatusChanged();

	// Also need to determine if the title needs formatting, since Initialise() is not called
	bTitleNeedsFormatting = USuqsProgression::GetTextNeedsFormatting(QuestDefinition->Title); 

	bIsLoading = false;
}

void USuqsQuestState::NotifyObjectiveStatusChanged()
{
	// Re-scan the objectives from top to bottom (this allows ANY change to have been made, including backtracking)
	// The next active objective is the next incomplete one in sequence which is on an active branch
	// If there is no next objective, then the quest is complete.
	int PrevObjIndex = CurrentObjectiveIndex;
	CurrentObjectiveIndex = -1;
	bool FailQuest = false;

	for (int i = 0; i < Objectives.Num(); ++i)
	{
		auto Obj = Objectives[i];
		// We ignore objectives not on the current branch entirely
		// The first objective that's on an active branch and incomplete is the current objective
		if (IsBranchActive(Obj->GetBranch()))
		{
			if (Obj->IsIncomplete())
			{
				CurrentObjectiveIndex = i;
				// first incomplete unfiltered objective is the next one 
				break;
			}
			else if (Obj->IsFailed())
			{
				// Failed objective may fail quest
				FailQuest = !Obj->GetContinueOnFail();
			}
		}
	}

	// If any unfiltered objectives failed, we lose
	if (FailQuest)
	{
	  ChangeStatus(ESuqsQuestStatus::Failed);
	}
  else if (CurrentObjectiveIndex == -1)
	{
		// No incomplete objectives, and no failures
		ChangeStatus(ESuqsQuestStatus::Completed);
	}
	else
	{
		// Just in case we go backwards (e.g. reset)
		ChangeStatus(ESuqsQuestStatus::Incomplete);

		if (PrevObjIndex != CurrentObjectiveIndex)
		{
			if (!bSuppressObjectiveChangeEvent)
			{
				// Raise task removed for any non-hidden tasks, these would not have received a notification before,
				// either because they're incomplete (optional), or non-sequential
				// from the previous objective
				// This is for convenience
				if (Objectives.IsValidIndex(PrevObjIndex))
				{
					const auto PrevObj = Objectives[PrevObjIndex];
					for (const auto PrevTask : PrevObj->GetTasks())
					{
						if (!PrevTask->GetHidden())
						{
							Progression->RaiseTaskRemoved(PrevTask);
						}
					}
				}
				Progression->RaiseCurrentObjectiveChanged(this);
			}
		}
	}
}

void USuqsQuestState::OverrideStatus(ESuqsQuestStatus OverrideStatus)
{
	ChangeStatus(OverrideStatus);
}

void USuqsQuestState::NotifyGateOpened(const FName& GateName)
{
	// This one proceeds downards to children
	// Cascade first so that objectives & tasks are finished first
	for (auto Obj : Objectives)
	{
		Obj->NotifyGateOpened(GateName);
	}

	if (IsResolveBlockedOn(ESuqsResolveBarrierCondition::Gate) && ResolveBarrier.Gate == GateName) {
	  MaybeNotifyStatusChange();
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
		case ESuqsQuestStatus::Failed:
			Progression->RaiseQuestFailed(this);
			break;
		case ESuqsQuestStatus::Incomplete:
			Progression->RaiseQuestReset(this);
			break;
		default: break;
		}

		QueueStatusChangeNotification();
	}
}

void USuqsQuestState::QueueStatusChangeNotification()
{
	ResolveBarrier = Progression->GetResolveBarrierForQuest(QuestDefinition, Status);

	// May immediately be satisfied
	MaybeNotifyStatusChange();
}

bool USuqsQuestState::IsResolveBlockedOn(ESuqsResolveBarrierCondition Barrier) const
{
	return ResolveBarrier.bPending &&
	   (ResolveBarrier.Conditions & static_cast<uint32>(Barrier)) > 0;
}

void USuqsQuestState::MaybeNotifyStatusChange()
{
	// Early-out if incomplete or barrier has already been processed so we only do this once per status change
	if (!ResolveBarrier.bPending)
		return;

	// Can't resolve unless completed/failed
	if (!IsCompleted() && !IsFailed())
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
		Progression->ProcessQuestStatusChange(this);
		ResolveBarrier.bPending = false;
	}
}

//PRAGMA_ENABLE_OPTIMIZATION