#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine.h"
#include "SuqsPlayState.h"
#include "TestQuestData.h"


class USuqsPlayState;
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestAcceptSimple, "SUQSTest.QuestAcceptSimple",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestAcceptSimple::RunTest(const FString& Parameters)
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
	TestEqual("Side quest should be unavailable", PlayState->GetQuestStatus("Q_Side1"), ESuqsQuestStatus::Unavailable);
	TestTrue("Should be able to accept main quest", PlayState->AcceptQuest("Q_Main1"));
	TestEqual("Main quest should be incomplete", PlayState->GetQuestStatus("Q_Main1"), ESuqsQuestStatus::Incomplete);
	TestTrue("Should be able to accept side quest", PlayState->AcceptQuest("Q_Side1"));
	TestEqual("Side quest should be incomplete", PlayState->GetQuestStatus("Q_Side1"), ESuqsQuestStatus::Incomplete);
	TestFalse("Accepting Main quest again should do nothing", PlayState->AcceptQuest("Q_Main1"));

	
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestAcceptFailedComplete, "SUQSTest.QuestAcceptFailedComplete",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestAcceptFailedComplete::RunTest(const FString& Parameters)
{
	USuqsPlayState* PlayState = NewObject<USuqsPlayState>();

	UDataTable* QuestTable = NewObject<UDataTable>();
	QuestTable->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable->CreateTableFromJSONString(SmallestPossibleQuestJson);

	PlayState->QuestDataTables.Add(QuestTable);

	// Test accepting failed
	TestTrue("Accept smallest quest", PlayState->AcceptQuest("Q_Smol"));
	TestEqual("Smol quest should be incomplete", PlayState->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Incomplete);
	PlayState->FailQuest("Q_Smol");
	TestEqual("Smol quest should now be failed", PlayState->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Failed);
	TestTrue("Accepting failed quest should succeeed (and reset)", PlayState->AcceptQuest("Q_Smol"));
	TestEqual("Smol quest should be incomplete again", PlayState->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Incomplete);
	// remove so we can go again
	PlayState->RemoveQuest("Q_Smol");

	// Test accepting completed
	TestTrue("Accept smallest quest", PlayState->AcceptQuest("Q_Smol"));
	TestEqual("Smol quest should be incomplete", PlayState->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Incomplete);
	PlayState->CompleteTask("Q_Smol", "T_Smol");
	TestEqual("Smol quest should now be completed", PlayState->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Completed);
	TestFalse("Accepting completed quest should do nothing by default", PlayState->AcceptQuest("Q_Smol"));
	TestEqual("Smol quest should still be completed", PlayState->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Completed);
	TestTrue("Accepting completed quest with reset option should reset", PlayState->AcceptQuest("Q_Smol", false, true));
	TestEqual("Smol quest should be incomplete again", PlayState->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Incomplete);

	
	
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
