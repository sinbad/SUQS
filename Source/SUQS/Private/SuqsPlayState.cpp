#include "SuqsPlayState.h"
#include "SuqsObjectiveState.h"
#include "SuqsQuestState.h"
#include "SuqsTaskState.h"


DEFINE_LOG_CATEGORY(LogSuqsPlayState)

void USuqsPlayState::EnsureQuestDefinitionsBuilt()
{
	// Build unified quest table
	if (QuestDefinitions.Num() == 0 && QuestDataTables.Num() > 0)
	{
		for (auto Table : QuestDataTables)
		{
			Table->ForeachRow<FSuqsQuest>("", [this, Table](const FName& Key, const FSuqsQuest& Quest)
            {
                if (QuestDefinitions.Contains(Quest.Identifier))
                	UE_LOG(LogSuqsPlayState, Error, TEXT("Quest ID '%s' has been used more than once! Duplicate entry was in %s"), *Quest.Identifier.ToString(), *Table->GetName());

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
				
                QuestDefinitions.Add(Quest.Identifier, Quest);
            });
		}
	}
}

ESuqsQuestStatus USuqsPlayState::GetQuestStatus(FName QuestID) const
{
	// Could make a lookup for this, but we'd need to post-load call to re-populate it, leave for now
	const auto State = FindQuestState(QuestID);

	if (State)
		return State->GetStatus();
	else
		return ESuqsQuestStatus::Unavailable;
	
}

USuqsQuestState* USuqsPlayState::GetQuest(FName QuestID)
{
	auto Status = FindQuestState(QuestID);
	return Status;
}


USuqsQuestState* USuqsPlayState::FindQuestState(const FName& QuestID)
{
	auto PQ = ActiveQuests.Find(QuestID);
	if (PQ)
		return *PQ;
	PQ = QuestArchive.Find(QuestID);
	if (PQ)
		return *PQ;

	return nullptr;
	
}

const USuqsQuestState* USuqsPlayState::FindQuestState(const FName& QuestID) const
{
	return const_cast<USuqsPlayState*>(this)->FindQuestState(QuestID);
}

USuqsTaskState* USuqsPlayState::FindTaskStatus(const FName& QuestID, const FName& TaskID)
{
	auto Q = FindQuestState(QuestID);
	if (Q)
	{
		return Q->FindTask(TaskID);
	}
	
	return nullptr;
}


void USuqsPlayState::GetAcceptedQuestIdentifiers(TArray<FName>& AcceptedQuestIDsOut) const
{
	ActiveQuests.GenerateKeyArray(AcceptedQuestIDsOut);
}

void USuqsPlayState::GetArchivedQuestIdentifiers(TArray<FName>& ArchivedQuestIDsOut) const
{
	QuestArchive.GenerateKeyArray(ArchivedQuestIDsOut);
}


void USuqsPlayState::GetAcceptedQuests(TArray<USuqsQuestState*>& AcceptedQuestsOut) const
{
	ActiveQuests.GenerateValueArray(AcceptedQuestsOut);
}

void USuqsPlayState::GetArchivedQuests(TArray<USuqsQuestState*>& ArchivedQuestsOut) const
{
	QuestArchive.GenerateValueArray(ArchivedQuestsOut);
}

bool USuqsPlayState::AcceptQuest(FName QuestID, bool bResetIfFailed, bool bResetIfComplete, bool bResetIfInProgress)
{
	EnsureQuestDefinitionsBuilt();

	auto QDef = QuestDefinitions.Find(QuestID);
	if (QDef)
	{
		// Check that we don't already have this quest
		auto Quest = FindQuestState(QuestID);
		if (Quest)
		{
			bool bReset;
			switch(Quest->Status)
			{
			case ESuqsQuestStatus::Incomplete:
				bReset = bResetIfInProgress;
				break;
			case ESuqsQuestStatus::Completed: 
				bReset = bResetIfComplete;
				break;
			case ESuqsQuestStatus::Failed: 
				bReset = bResetIfFailed;
				break;
			default:
				bReset = true;
				break;
			}

			if (!bReset)
			{
				UE_LOG(LogSuqsPlayState, Warning, TEXT("Ignoring request to accept quest %s because it has status %d"), *QuestID.ToString(), Quest->Status);
				return false;
			}

			Quest->Reset();
		}
		else
		{
			// New quest
			Quest = NewObject<USuqsQuestState>(this);
			Quest->Initialise(QDef, this);
			ActiveQuests.Add(QuestID, Quest);
		}

		OnQuestAccepted.Broadcast(Quest);
		return true;
	}
	else
	{
		UE_LOG(LogSuqsPlayState, Error , TEXT("Attempted to accept a non-existent quest %s"), *QuestID.ToString());
		return false;
	}
	
}


void USuqsPlayState::ResetQuest(FName QuestID)
{
	auto Q = FindQuestState(QuestID);
	if (Q)
		Q->Reset();

	// note we don't update quest lists here, but we rely on callbacks since you could reset or re-activate using the quest itself
	
}

void USuqsPlayState::FailQuest(FName QuestID)
{
	auto Q = FindQuestState(QuestID);
	if (Q)
		Q->Fail();

	// note we don't update quest lists here, but we rely on callbacks since you could fail via the quest itself
}

void USuqsPlayState::FailTask(FName QuestID, FName TaskIdentifier)
{
	auto T = FindTaskStatus(QuestID, TaskIdentifier);
	if (T)
	{
		T->Fail();
	}
}


void USuqsPlayState::CompleteTask(FName QuestID, FName TaskIdentifier)
{
	auto T = FindTaskStatus(QuestID, TaskIdentifier);
	if (T)
	{
		T->Complete();
	}
}

void USuqsPlayState::ProgressTask(FName QuestID, FName TaskIdentifier, int Delta)
{
	auto T = FindTaskStatus(QuestID, TaskIdentifier);
	if (T)
	{
		T->Progress(Delta);
	}
}


void USuqsPlayState::RaiseTaskUpdated(USuqsTaskState* Task)
{
	// might be worth queuing these up and raising combined?
	OnTaskUpdated.Broadcast(Task);
}

void USuqsPlayState::RaiseTaskCompleted(USuqsTaskState* Task)
{
	OnTaskCompleted.Broadcast(Task);
}
void USuqsPlayState::RaiseTaskFailed(USuqsTaskState* Task)
{
	OnTaskFailed.Broadcast(Task);
}


void USuqsPlayState::RaiseObjectiveCompleted(USuqsObjectiveState* Objective)
{
	OnObjectiveCompleted.Broadcast(Objective);
}

void USuqsPlayState::RaiseObjectiveFailed(USuqsObjectiveState* Objective)
{
	OnObjectiveFailed.Broadcast(Objective);
}


void USuqsPlayState::RaiseQuestCompleted(USuqsQuestState* Quest)
{
	// Move quest to the correct list
	ActiveQuests.Remove(Quest->GetIdentifier());
	QuestArchive.Add(Quest->GetIdentifier(), Quest);

	OnQuestCompleted.Broadcast(Quest);

	// TODO: trigger the acceptance of quests which depend on this completion
}

void USuqsPlayState::RaiseQuestFailed(USuqsQuestState* Quest)
{
	// Move quest to the correct list
	ActiveQuests.Remove(Quest->GetIdentifier());
	QuestArchive.Add(Quest->GetIdentifier(), Quest);

	OnQuestFailed.Broadcast(Quest);

	// TODO: trigger the acceptance of quests which depend on this failure
}

void USuqsPlayState::RaiseQuestReset(USuqsQuestState* Quest)
{
	// Move quest to the correct list
	QuestArchive.Remove(Quest->GetIdentifier());
	ActiveQuests.Add(Quest->GetIdentifier(), Quest);
	OnQuestAccepted.Broadcast(Quest);
}

// FTickableGameObject start
void USuqsPlayState::Tick(float DeltaTime)
{
	for (auto& QuestPair : ActiveQuests)
	{
		QuestPair.Value->Tick(DeltaTime);
	}
}

TStatId USuqsPlayState::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USuqsStatus, STATGROUP_Tickables);
}
// FTickableGameObject end
