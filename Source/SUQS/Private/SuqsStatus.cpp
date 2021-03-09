#include "SuqsStatus.h"


DEFINE_LOG_CATEGORY(LogSuqsState)

ESuqsSummaryState USuqsStatus::GetQuestState(const FName& Name) const
{
	// Could make a lookup for this, but we'd need to post-load call to re-populate it, leave for now
	auto Status = Quests.FindByPredicate([Name](const FSuqsQuestStatus& Status)
	{
		return Status.Name == Name;
	});

	if (Status)
		return Status->bState;
	else
		return ESuqsSummaryState::Unavailable;
	
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
	// TODO
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
	// TODO: advance all timed quest tasks, fail if exceeded
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
				UE_LOG(LogSuqsState, Error, TEXT("Quest name '%s' has been used more than once! Second entry was in %s"), *Key.ToString(), *Table->GetName());
				
			QuestDefinitions.Add(Key, Quest);
		});
	}
	
}
