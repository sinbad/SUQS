#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine.h"
#include "SuqsQuest.h"
#include "TestQuestData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestCreateQuestDefinitions, "SUQSTest.CreateQuestDefinitions",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestCreateQuestDefinitions::RunTest(const FString& Parameters)
{
	UDataTable* QuestTable = NewObject<UDataTable>();

	QuestTable->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable->bIgnoreMissingFields = true;
	QuestTable->CreateTableFromJSONString(SimpleMainQuestJson);

	if (!TestEqual("Should have 2 quests in table", QuestTable->GetRowMap().Num(), 2))
		return false;

	return true;
}
