#include "SuqsPlayState.h"


DEFINE_LOG_CATEGORY(LogSuqsState)

ESuqsItemStatus USuqsPlayState::GetQuestState(const FName& Name) const
{
	// Could make a lookup for this, but we'd need to post-load call to re-populate it, leave for now
	const auto Status = FindQuestStatus(Name);

	if (Status)
		return Status->bState;
	else
		return ESuqsItemStatus::Unavailable;
	
}


FSuqsQuestState* USuqsPlayState::FindQuestStatus(const FName& QuestName)
{
	return Quests.FindByPredicate([QuestName](const FSuqsQuestState& Status)
    {
        return Status.Name == QuestName;
    });
}

const FSuqsQuestState* USuqsPlayState::FindQuestStatus(const FName& QuestName) const
{
	return Quests.FindByPredicate([QuestName](const FSuqsQuestState& Status)
    {
        return Status.Name == QuestName;
    });
}

FSuqsTaskState* USuqsPlayState::FindTaskStatus(const FName& QuestName, const FName& TaskID, FSuqsObjectiveState** OutObjective)
{
	if (auto Q = FindQuestStatus(QuestName))
		return FindTaskStatus(*Q, TaskID, OutObjective);
	
	return nullptr;
}

FSuqsTaskState* USuqsPlayState::FindTaskStatus(FSuqsQuestState& Q, const FName& TaskID, FSuqsObjectiveState** OutObjective)
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

void USuqsPlayState::ActivateQuest(const FName& Name)
{
	// TODO
}

void USuqsPlayState::FailQuest(const FName& Name)
{
	// TODO
}

void USuqsPlayState::FailTask(const FName& QuestName, const FName& TaskIdentifier)
{
	FSuqsQuestState* Q = FindQuestStatus(QuestName);
	if (Q)
	{
		FSuqsObjectiveState* Obj;
		auto T = FindTaskStatus(*Q, TaskIdentifier, &Obj);
		if (T)
		{
			FailTask(*Q, *Obj, *T);
		}
	}
}

void USuqsPlayState::FailTask(FSuqsQuestState& Q, FSuqsObjectiveState& O, FSuqsTaskState& T)
{
	const auto OldState = T.bState;
	T.bState = ESuqsItemStatus::Failed;
	TaskStateChanged(OldState, Q, O, T);
}

void USuqsPlayState::TaskStateChanged(ESuqsItemStatus PrevState, FSuqsQuestState& Quest,
	FSuqsObjectiveState& Objective, FSuqsTaskState& Task)
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


void USuqsPlayState::CompleteTask(const FName& QuestName, const FName& TaskIdentifier)
{
	// TODO
}

void USuqsPlayState::ProgressTask(const FName& QuestName, const FName& TaskIdentifier, int Delta)
{
	// TODO
}


void USuqsPlayState::SetTaskHidden(const FName& QuestName, const FName& TaskIdentifier, bool bHidden)
{
	// TODO
}

// FTickableGameObject start
void USuqsPlayState::Tick(float DeltaTime)
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

TStatId USuqsPlayState::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USuqsStatus, STATGROUP_Tickables);
}
// FTickableGameObject end

void USuqsPlayState::PostLoad()
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
