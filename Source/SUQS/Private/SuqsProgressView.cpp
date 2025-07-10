
#include "SuqsProgressView.h"

#include "SuqsProgression.h"

FSuqsTaskStateView::FSuqsTaskStateView(): CompletedNumber(0), bHidden(false)
{
}

void FSuqsTaskStateView::FromUObject(USuqsTaskState* State)
{
	Identifier = State->GetIdentifier();
	Title = State->GetTitle();
	bMandatory = State->IsMandatory();
	TargetNumber = State->GetTargetNumber();
	CompletedNumber = State->GetNumber();
	TimeRemaining = State->GetTimeRemaining();
	Status = State->GetStatus();
	bHidden = State->GetHidden();
	
}

FSuqsQuestStateView::FSuqsQuestStateView()
{
}

void FSuqsQuestStateView::FromUObject(USuqsQuestState* State, bool bIncludeCompletedObjectives)
{
	Identifier = State->GetIdentifier();
	Labels = State->GetLabels();
	Title = State->GetTitle();
	Status = State->GetStatus();

	Description = State->GetDescription();

	auto CurrObj = State->GetCurrentObjective();

	if (CurrObj)
	{
		CurrentObjectiveIdentifier = CurrObj->GetIdentifier();
		CurrentObjectiveDescription = CurrObj->GetDescription();
	}
	else
	{
		CurrentObjectiveIdentifier = NAME_None;
		CurrentObjectiveDescription = FText::GetEmpty();
	}


	TArray<USuqsObjectiveState*> ObjectivesOfInterest;
	if (bIncludeCompletedObjectives)
	{
		State->GetActiveObjectives(ObjectivesOfInterest);
	}
	else
	{
		ObjectivesOfInterest.Add(CurrObj);
	}
	
	CurrentTasks.Reset();
	TArray<USuqsTaskState*> RelevantTasks;
	for (auto& Obj : ObjectivesOfInterest)
	{
		if (Obj == CurrObj || (bIncludeCompletedObjectives && !Obj->IsIncomplete()))
		{
			Obj->GetAllRelevantTasks(RelevantTasks);
			for (USuqsTaskState* TaskState : RelevantTasks)
			{
				auto& Task = CurrentTasks.AddDefaulted_GetRef();
				Task.FromUObject(TaskState);
			}
		}
	}

}

FSuqsProgressView::FSuqsProgressView()
{
}

void FSuqsProgressView::FromUObject(USuqsProgression* State, bool bIncludeCompletedObjectives)
{
	TArray<USuqsQuestState*> QuestStates;
	State->GetAcceptedQuests(QuestStates);
	ActiveQuests.Reset(QuestStates.Num());

	for (USuqsQuestState* QuestState : QuestStates)
	{
		if (QuestState->IsPlayerVisible())
		{
			auto& Quest = ActiveQuests.AddDefaulted_GetRef();
			Quest.FromUObject(QuestState, bIncludeCompletedObjectives);
		}
	}
	// If not player visible, ignore quest
}


bool USuqsProgressViewHelpers::GetProgressViewDifferences(const FSuqsProgressView& Before,
	const FSuqsProgressView& After,
	FSuqsProgressViewDiff& OutDiff)
{
	OutDiff.Entries.Reset();

	bool bAnyChanges = false;
	// Check for added / modified quests
	for (const auto& NewQ : After.ActiveQuests)
	{
		const FSuqsQuestStateView* PrevQ = nullptr;
		for (const auto& OldQ : Before.ActiveQuests)
		{
			if (OldQ.Identifier == NewQ.Identifier)
			{
				PrevQ = &OldQ;
				break;
			}
		}

		if (PrevQ)
		{
			// Existing quest
			// Check complete/failed change
			if (NewQ.Status != PrevQ->Status &&
				(NewQ.Status == ESuqsQuestStatus::Completed || NewQ.Status ==
					ESuqsQuestStatus::Failed))
			{
				auto& Entry = OutDiff.Entries.AddDefaulted_GetRef();
				Entry.Category = ESuqsProgressViewDiffCategory::Quest;
				Entry.ChangeType = NewQ.Status == ESuqsQuestStatus::Completed ? ESuqsProgressViewDiffChangeType::Completed : ESuqsProgressViewDiffChangeType::Failed;
				Entry.QuestID = NewQ.Identifier;
				bAnyChanges = true;
				
			}
			else if (PrevQ->IsModified(NewQ))
			{
				auto& Entry = OutDiff.Entries.AddDefaulted_GetRef();
				Entry.Category = ESuqsProgressViewDiffCategory::Quest;
				Entry.ChangeType = ESuqsProgressViewDiffChangeType::Modified;
				Entry.QuestID = NewQ.Identifier;
				bAnyChanges = true;
			}

			// Now check task changes
			for (const auto& NewT : NewQ.CurrentTasks)
			{
				const FSuqsTaskStateView* PrevT = nullptr;
				for (const auto& OldT : PrevQ->CurrentTasks)
				{
					if (OldT.Identifier == NewT.Identifier)
					{
						PrevT = &OldT;
						break;
					}
				}

				if (PrevT)
				{
					// Existing task
					// Check complete/failed changes first
					if (NewT.Status != PrevT->Status &&
						(NewT.Status == ESuqsTaskStatus::Completed || NewT.Status == ESuqsTaskStatus::Failed))
					{
						auto& Entry = OutDiff.Entries.AddDefaulted_GetRef();
						Entry.Category = ESuqsProgressViewDiffCategory::Task;
						Entry.ChangeType = NewT.Status == ESuqsTaskStatus::Completed ? ESuqsProgressViewDiffChangeType::Completed : ESuqsProgressViewDiffChangeType::Failed;
						Entry.QuestID = NewQ.Identifier;
						Entry.TaskID = NewT.Identifier;
						bAnyChanges = true;
				
					}
					else if (PrevT->IsModified(NewT))
					{
						auto& Entry = OutDiff.Entries.AddDefaulted_GetRef();
						Entry.Category = ESuqsProgressViewDiffCategory::Task;
						Entry.ChangeType = ESuqsProgressViewDiffChangeType::Modified;
						Entry.QuestID = NewQ.Identifier;
						Entry.TaskID = NewT.Identifier;
						bAnyChanges = true;
					}
					
				}
				else
				{
					// New task
					auto& Entry = OutDiff.Entries.AddDefaulted_GetRef();
					Entry.Category = ESuqsProgressViewDiffCategory::Task;
					Entry.ChangeType = ESuqsProgressViewDiffChangeType::Added;
					Entry.QuestID = NewQ.Identifier;
					Entry.TaskID = NewT.Identifier;

					bAnyChanges = true;

				}
			}

			// Check deleted tasks
			for (const auto& OldT : PrevQ->CurrentTasks)
			{
				bool bFound = false;
				for (const auto& NewT : NewQ.CurrentTasks)
				{
					if (OldT.Identifier == NewT.Identifier)
					{
						bFound = true;
						break;
					}
				}

				if (!bFound)
				{
					// Removed task
					auto& Entry = OutDiff.Entries.AddDefaulted_GetRef();
					Entry.Category = ESuqsProgressViewDiffCategory::Task;
					Entry.ChangeType = ESuqsProgressViewDiffChangeType::Removed;
					Entry.QuestID = PrevQ->Identifier;
					Entry.TaskID = OldT.Identifier;
			
					bAnyChanges = true;
					
				}
			}
		}
		else
		{
			// New quest
			{
				auto& Entry = OutDiff.Entries.AddDefaulted_GetRef();
				Entry.Category = ESuqsProgressViewDiffCategory::Quest;
				Entry.ChangeType = ESuqsProgressViewDiffChangeType::Added;
				Entry.QuestID = NewQ.Identifier;
			}

			// Also add each task, for consistency
			for (const FSuqsTaskStateView& NewT : NewQ.CurrentTasks)
			{
				// New task
				auto& Entry = OutDiff.Entries.AddDefaulted_GetRef();
				Entry.Category = ESuqsProgressViewDiffCategory::Task;
				Entry.ChangeType = ESuqsProgressViewDiffChangeType::Added;
				Entry.QuestID = NewQ.Identifier;
				Entry.TaskID = NewT.Identifier;
			}

			bAnyChanges = true;
		}
		
	}

	// Check for quest removals
	for (const auto& OldQ : Before.ActiveQuests)
	{
		bool bFound = false;
		for (const auto& NewQ : After.ActiveQuests)
		{
			if (OldQ.Identifier == NewQ.Identifier)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			// Removed quest
			auto& Entry = OutDiff.Entries.AddDefaulted_GetRef();
			Entry.Category = ESuqsProgressViewDiffCategory::Quest;
			Entry.ChangeType = ESuqsProgressViewDiffChangeType::Removed;
			Entry.QuestID = OldQ.Identifier;
			
			bAnyChanges = true;
		}
	}
	return bAnyChanges;
}

void USuqsProgressViewHelpers::GetProgressViewDifferencesBP(const FSuqsProgressView& Before,
	const FSuqsProgressView& After,
	FSuqsProgressViewDiff& Differences,
	bool& bWasDifferent)
{
	bWasDifferent = GetProgressViewDifferences(Before, After, Differences);
}

void USuqsProgressViewHelpers::GetQuestStateFromProgressView(const FSuqsProgressView& ProgressView,
	FName QuestID,
	FSuqsQuestStateView& Quest,
	bool& bWasFound)
{
	for (const FSuqsQuestStateView& Q : ProgressView.ActiveQuests)
	{
		if (Q.Identifier == QuestID)
		{
			Quest = Q;
			bWasFound = true;
			return;
		}
	}
	bWasFound = false;
}

void USuqsProgressViewHelpers::GetTaskStateFromProgressView(const FSuqsProgressView& ProgressView,
                                                            FName QuestID,
                                                            FName TaskID,
                                                            FSuqsQuestStateView& Quest,
                                                            FSuqsTaskStateView& Task,
                                                            bool& bWasFound)
{
	GetQuestStateFromProgressView(ProgressView, QuestID, Quest, bWasFound);
	if (bWasFound)
	{
		for (const FSuqsTaskStateView& T : Quest.CurrentTasks)
		{
			if (T.Identifier == TaskID)
			{
				Task = T;
				return;
			}
		}
		bWasFound = false;
	}
}

