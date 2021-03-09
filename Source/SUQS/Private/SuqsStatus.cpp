#include "SuqsStatus.h"


DEFINE_LOG_CATEGORY(LogSuqsState)

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
