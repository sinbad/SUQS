#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine.h"
#include "SuqsPlayState.h"
#include "TestQuestData.h"


class USuqsPlayState;
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestAccept, "SUQSTest.QuestAccept",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestAccept::RunTest(const FString& Parameters)
{
	USuqsPlayState* PlayState = NewObject<USuqsPlayState>();
	UDataTable* QuestTable1 = NewObject<UDataTable>();
	QuestTable1->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable1->CreateTableFromJSONString(SimpleMainQuestJson);

	UDataTable* QuestTable2 = NewObject<UDataTable>();
	QuestTable2->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable2->CreateTableFromJSONString(SimpleSideQuestJson);

	PlayState->QuestDataTables.Add(QuestTable1);
	PlayState->QuestDataTables.Add(QuestTable2);	

	TestEqual("Main quest should be unavailable", PlayState->GetQuestStatus("Q_Main1"), ESuqsQuestStatus::Unavailable);

	TestTrue("Should be able to accept main quest", PlayState->AcceptQuest("Q_Main1"));

	TestEqual("Main quest should be incomplete", PlayState->GetQuestStatus("Q_Main1"), ESuqsQuestStatus::Incomplete);
	
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestReset, "SUQSTest.QuestReset",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::ClientContext |
    EAutomationTestFlags::ProductFilter)

bool FTestQuestReset::RunTest(const FString& Parameters)
{
	return true;
}
