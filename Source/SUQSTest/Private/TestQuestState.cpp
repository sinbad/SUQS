#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine.h"
#include "SuqsObjectiveState.h"
#include "SuqsPlayState.h"
#include "SuqsTaskState.h"
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
	USuqsPlayState* PlayState = NewObject<USuqsPlayState>();
	UDataTable* QuestTable = NewObject<UDataTable>();
	QuestTable->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable->CreateTableFromJSONString(SimpleMainQuestJson);

	PlayState->QuestDataTables.Add(QuestTable);

	TestTrue("Should be able to accept main quest", PlayState->AcceptQuest("Q_Main1"));
	TestEqual("Main quest should be incomplete", PlayState->GetQuestStatus("Q_Main1"), ESuqsQuestStatus::Incomplete);

	auto Q = PlayState->GetQuest("Q_Main1");
	auto Obj = Q->GetCurrentObjective();
	TestNotNull("Current Objective should be valid", Obj);
	TestEqual("O1 objective should be current", Obj->GetIdentifier(), FName("O1"));
	TestEqual("O1 objective should be not started", Obj->GetStatus(), ESuqsObjectiveStatus::NotStarted);

	// Complete first task via top-level API
	PlayState->CompleteTask("Q_Main1", "T_ReachThePlace");
	TestEqual("Main quest should still be incomplete", PlayState->GetQuestStatus("Q_Main1"), ESuqsQuestStatus::Incomplete);
	// Now let's do things via the object interface
	Obj = Q->GetCurrentObjective();
	TestNotNull("Current Objective should be valid", Obj);
	TestEqual("O1 objective should be current", Obj->GetIdentifier(), FName("O1"));
	TestEqual("O1 objective should be in progress", Obj->GetStatus(), ESuqsObjectiveStatus::InProgress);

	auto Tasks = Obj->GetTasks();
	TestEqual("Should be correct number of tasks", Tasks.Num(), 3);
	TestEqual("First task should be complete", Tasks[0]->GetStatus(), ESuqsTaskStatus::Completed);
	TestEqual("Second task should be not started", Tasks[1]->GetStatus(), ESuqsTaskStatus::NotStarted);
	TestEqual("Third task should be not started", Tasks[2]->GetStatus(), ESuqsTaskStatus::NotStarted);

	// now complete the other 2 tasks, but via the bottom level API
	Tasks[1]->Complete();
	Tasks[2]->Complete();
	// This should now complete one objective and set the next one to be active

	TestEqual("First task should be complete", Tasks[0]->GetStatus(), ESuqsTaskStatus::Completed);
	TestEqual("Second task should be complete", Tasks[1]->GetStatus(), ESuqsTaskStatus::Completed);
	TestEqual("Third task should be complete", Tasks[2]->GetStatus(), ESuqsTaskStatus::Completed);
	TestEqual("O1 objective should be complete", Obj->GetStatus(), ESuqsObjectiveStatus::Completed);
	Obj = Q->GetCurrentObjective();
	TestNotNull("Current Objective should be valid", Obj);
	TestEqual("O2 objective should be current", Obj->GetIdentifier(), FName("O2"));
	TestEqual("O2 objective should be not started", Obj->GetStatus(), ESuqsObjectiveStatus::NotStarted);

	// We'll complete one task of the objective
	Obj->GetTasks()[0]->Complete();
	TestEqual("O2 objective should now be in progress", Obj->GetStatus(), ESuqsObjectiveStatus::InProgress);
	TestEqual("O2 first task should be done", Obj->GetTasks()[0]->GetStatus(), ESuqsTaskStatus::Completed);

	// Now we reset
	Q->Reset();
	
	Obj = Q->GetCurrentObjective();
	TestNotNull("Current Objective should be valid", Obj);
	TestEqual("O1 objective should be current again after reset", Obj->GetIdentifier(), FName("O1"));
	TestEqual("O1 objective should be not started again", Obj->GetStatus(), ESuqsObjectiveStatus::NotStarted);
	Tasks = Obj->GetTasks();
	TestEqual("First task should be not started", Tasks[0]->GetStatus(), ESuqsTaskStatus::NotStarted);
	TestEqual("Second task should be not started", Tasks[1]->GetStatus(), ESuqsTaskStatus::NotStarted);
	TestEqual("Third task should be not started", Tasks[2]->GetStatus(), ESuqsTaskStatus::NotStarted);

	TestEqual("O2 objective should be not started again", Q->GetObjectives()[1]->GetStatus(), ESuqsObjectiveStatus::NotStarted);
	TestEqual("O2 first task should be not started again", Obj->GetTasks()[0]->GetStatus(), ESuqsTaskStatus::NotStarted);
	
	
	return true;
}
