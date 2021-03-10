#include "SuqsPlayState.h"


#include "SuqsObjectiveState.h"
#include "SuqsQuestState.h"
#include "SuqsTaskState.h"


DEFINE_LOG_CATEGORY(LogSuqsPlayState)

void USuqsPlayState::EnsureStateBuilt()
{
	EnsureQuestDefinitionsBuilt();
}


void USuqsPlayState::EnsureQuestDefinitionsBuilt()
{
	// Build unified quest table
	if (QuestDefinitions.Num() == 0 && QuestDataTables.Num() > 0)
	{
		for (auto Table : QuestDataTables)
		{
			Table->ForeachRow<FSuqsQuest>("", [this, Table](const FName& Key, const FSuqsQuest& Quest)
            {
                if (QuestDefinitions.Contains(Key))
                	UE_LOG(LogSuqsPlayState, Error, TEXT("Quest name '%s' has been used more than once! Duplicate entry was in %s"), *Key.ToString(), *Table->GetName());

                // Check task IDs are unique
                TSet<FName> TaskIDSet;
                for (auto& Objective : Quest.Objectives)
                {
                    for (auto& Task : Objective.Tasks)
                    {
                        bool bDuplicate;
                        TaskIDSet.Add(Task.Identifier, &bDuplicate);
                        if (bDuplicate)
                        	UE_LOG(LogSuqsPlayState, Error, TEXT("Task ID '%s' has been used more than once! Duplicate entry title: %s"), *Task.Identifier.ToString(), *Task.Title.ToString());
                    }
                }
				
                QuestDefinitions.Add(Key, Quest);
            });
		}
	}
}

ESuqsQuestStatus USuqsPlayState::GetQuestState(const FName& Name) const
{
	// Could make a lookup for this, but we'd need to post-load call to re-populate it, leave for now
	const auto Status = FindQuestStatus(Name);

	if (Status)
		return Status->GetStatus();
	else
		return ESuqsQuestStatus::Unavailable;
	
}


USuqsQuestState* USuqsPlayState::FindQuestStatus(const FName& QuestName)
{
	return QuestState.FindChecked(QuestName);
}

const USuqsQuestState* USuqsPlayState::FindQuestStatus(const FName& QuestName) const
{
	return QuestState.FindChecked(QuestName);
}

USuqsTaskState* USuqsPlayState::FindTaskStatus(const FName& QuestName, const FName& TaskID)
{
	auto Q = FindQuestStatus(QuestName);
	if (Q)
	{
		return Q->FindTask(TaskID);
	}
	
	return nullptr;
}

void USuqsPlayState::AcceptQuest(const FName& Name)
{
	// TODO
}

void USuqsPlayState::FailQuest(const FName& Name)
{
	// TODO
}

void USuqsPlayState::FailTask(const FName& QuestName, const FName& TaskIdentifier)
{
	auto T = FindTaskStatus(QuestName, TaskIdentifier);
	if (T)
	{
		T->Fail();
	}
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
	for (auto& QuestPair : QuestState)
	{
		QuestPair.Value->Tick(DeltaTime);
	}
}

TStatId USuqsPlayState::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USuqsStatus, STATGROUP_Tickables);
}
// FTickableGameObject end
