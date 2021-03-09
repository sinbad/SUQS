#include "SuqsStatus.h"


DEFINE_LOG_CATEGORY(LogSuqsState)

ESuqsSummaryState USuqsStatus::GetQuestState(const FName& Name) const
{
	// Could make a lookup for this, but we'd need to post-load call to re-populate it, leave for now
	const auto Status = FindQuestStatus(Name);

	if (Status)
		return Status->bState;
	else
		return ESuqsSummaryState::Unavailable;
	
}


FSuqsQuestStatus* USuqsStatus::FindQuestStatus(const FName& QuestName)
{
	return Quests.FindByPredicate([QuestName](const FSuqsQuestStatus& Status)
    {
        return Status.Name == QuestName;
    });
}

const FSuqsQuestStatus* USuqsStatus::FindQuestStatus(const FName& QuestName) const
{
	return Quests.FindByPredicate([QuestName](const FSuqsQuestStatus& Status)
    {
        return Status.Name == QuestName;
    });
}

FSuqsTaskStatus* USuqsStatus::FindTaskStatus(const FName& QuestName, const FName& TaskID, FSuqsObjectiveStatus** OutObjective)
{
	if (auto Q = FindQuestStatus(QuestName))
		return FindTaskStatus(*Q, TaskID, OutObjective);
	
	return nullptr;
}

FSuqsTaskStatus* USuqsStatus::FindTaskStatus(FSuqsQuestStatus& Q, const FName& TaskID, FSuqsObjectiveStatus** OutObjective)
{
	// Task IDs are unique by quest 
	for (auto& Objective : Q.Objectives)
	{
		for (auto& Task : Objective.Tasks)
		{
			if (Task.Identifier == TaskID)
			{
				if (OutObjective)
					*OutObjective = &Objective;
				return &Task; 
			}
		}
	}
	return nullptr;
}

void USuqsStatus::ActivateQuest(const FName& Name)
{
	// TODO
}

void USuqsStatus::FailQuest(const FName& Name)
{
	// TODO
}

void USuqsStatus::FailTask(const FName& QuestName, const FName& TaskIdentifier)
{
	FSuqsQuestStatus* Q = FindQuestStatus(QuestName);
	if (Q)
	{
		FSuqsObjectiveStatus* Obj;
		auto T = FindTaskStatus(*Q, TaskIdentifier, &Obj);
		if (T)
		{
			FailTask(*Q, *Obj, *T);
		}
	}
}

void USuqsStatus::FailTask(FSuqsQuestStatus& Q, FSuqsObjectiveStatus& O, FSuqsTaskStatus& T)
{
	const auto OldState = T.bState;
	T.bState = ESuqsSummaryState::Failed;
	TaskStateChanged(OldState, Q, O, T);
}

void USuqsStatus::TaskStateChanged(ESuqsSummaryState PrevState, FSuqsQuestStatus& Quest,
	FSuqsObjectiveStatus& Objective, FSuqsTaskStatus& Task)
{
	// Get definitions of quest / objective
	// Don't assume the indexing is the same, it should be buuuuut it might have changed
	// ARRRGH
	// OK it's not the index of the statuses we should care about but the indexes of the definitions, those will
	// determine what happens next
	// So I need to change all this, bugger
	// What I really need to do is apply the status data over the top of the master definitions and see what shakes out next
	// So probably a complete re-examination of the def vs the status from top to bottom is better than trying to be
	// clever about which task has changed	
}


void USuqsStatus::CompleteTask(const FName& QuestName, const FName& TaskIdentifier)
{
	// TODO
}

void USuqsStatus::ProgressTask(const FName& QuestName, const FName& TaskIdentifier, int Delta)
{
	// TODO
}


void USuqsStatus::SetTaskHidden(const FName& QuestName, const FName& TaskIdentifier, bool bHidden)
{
	// TODO
}

// FTickableGameObject start
void USuqsStatus::Tick(float DeltaTime)
{
	for (auto& Quest : Quests)
	{
		for (auto& Objective : Quest.Objectives)
		{
			for (auto& Task : Objective.Tasks)
			{
				if (Task.bTimeLimit && Task.TimeRemaining > 0)
				{
					Task.TimeRemaining -= DeltaTime;
					if (Task.TimeRemaining <= 0)
					{
						Task.TimeRemaining = 0;
						FailTask(Quest, Objective, Task);
					}
				}
			}
		}
	}
}

TStatId USuqsStatus::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USuqsStatus, STATGROUP_Tickables);
}
// FTickableGameObject end

void USuqsStatus::PostLoad()
{
	Super::PostLoad();

	QuestDefinitions.Empty();
	for (auto Table : QuestDataTables)
	{
		Table->ForeachRow<FSuqsQuest>("", [this, Table](const FName& Key, const FSuqsQuest& Quest)
		{
			if (QuestDefinitions.Contains(Key))
				UE_LOG(LogSuqsState, Error, TEXT("Quest name '%s' has been used more than once! Duplicate entry was in %s"), *Key.ToString(), *Table->GetName());

			// Check task IDs are unique
			TSet<FName> TaskIDSet;
			for (auto& Objective : Quest.Objectives)
			{
				for (auto& Task : Objective.Tasks)
				{
					bool bDuplicate;
					TaskIDSet.Add(Task.Identifier, &bDuplicate);
					if (bDuplicate)
						UE_LOG(LogSuqsState, Error, TEXT("Task ID '%s' has been used more than once! Duplicate entry title: %s"), *Task.Identifier.ToString(), *Task.Title.ToString());
                }
			}
				
			QuestDefinitions.Add(Key, Quest);
		});
	}
	
}
