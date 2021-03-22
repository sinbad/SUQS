#include "Misc/AutomationTest.h"
#include "Engine.h"
#include "SuqsProgression.h"
#include "TestQuestData.h"
#include "CallbackCatcher.h"
#include "SuqsTaskState.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestTopLevelEvents, "SUQSTest.QuestTopLevelEvents",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestTopLevelEvents::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
		TArray<UDataTable*>{
			USuqsProgression::MakeQuestDataTableFromJSON(SimpleMainQuestJson)
		}
	);

	UCallbackCatcher* CallbackObj = NewObject<UCallbackCatcher>();
	CallbackObj->Subscribe(Progression);

	TestEqual("Should be no accepted to start", CallbackObj->AcceptedQuests.Num(), 0);
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_Main1"));
	TestEqual("Should have got one accept callback", CallbackObj->AcceptedQuests.Num(), 1);
	auto Q = Progression->GetQuest("Q_Main1");
	if (CallbackObj->AcceptedQuests.Num() > 0)
		TestEqual("Should have received correct quest callback", CallbackObj->AcceptedQuests[0], Q);

	TestEqual("Should be no failed to start", CallbackObj->FailedQuests.Num(), 0);
	Progression->GetNextMandatoryTask("Q_Main1")->Fail();
	TestEqual("Should be a failed callback", CallbackObj->FailedQuests.Num(), 1);
	if (CallbackObj->FailedQuests.Num() > 0)
		TestEqual("Should have received correct quest callback", CallbackObj->FailedQuests[0], Q);

	Q->Reset();
	TestEqual("Should have got another accept callback on reset", CallbackObj->AcceptedQuests.Num(), 2);
	if (CallbackObj->AcceptedQuests.Num() > 1)
		TestEqual("Should have received correct quest callback", CallbackObj->AcceptedQuests[1], Q);

	TestEqual("Should be no completed to start", CallbackObj->CompletedQuests.Num(), 0);
	Q->Complete();
	TestEqual("Should get quest completion callback", CallbackObj->CompletedQuests.Num(), 1);
	if (CallbackObj->CompletedQuests.Num() > 0)
		TestEqual("Should have received correct quest callback", CallbackObj->CompletedQuests[0], Q);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestDetailEvents, "SUQSTest.QuestDetailEvents",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestDetailEvents::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
        	USuqsProgression::MakeQuestDataTableFromJSON(SimpleMainQuestJson)
        }
    );

	UCallbackCatcher* CallbackObj = NewObject<UCallbackCatcher>();
	CallbackObj->Subscribe(Progression);
	
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_Main1"));
	auto Q = Progression->GetQuest("Q_Main1");
	TestTrue("Task complete should work", Progression->CompleteTask("Q_Main1", "T_ReachThePlace"));
	TestEqual("Should be a task update callback", CallbackObj->UpdatedTasks.Num(), 1);
	if (CallbackObj->UpdatedTasks.Num() > 0)
		TestEqual("Should have received correct task update callback", CallbackObj->UpdatedTasks[0], Q->GetTask("T_ReachThePlace"));
	TestEqual("Should be a task complete callback", CallbackObj->CompletedTasks.Num(), 1);
	if (CallbackObj->CompletedTasks.Num() > 0)
		TestEqual("Should have received correct task complete callback", CallbackObj->CompletedTasks[0], Q->GetTask("T_ReachThePlace"));
	Progression->ProgressTask("Q_Main1", "T_CollectDoobries", 1);
	// 2 update calls for this, one for the status change to "in progress", the other for the progress itself
	TestEqual("Should be 2 extra task update callback for progress", CallbackObj->UpdatedTasks.Num(), 3);
	if (CallbackObj->UpdatedTasks.Num() == 3)
	{
		TestEqual("Should have received correct task update callback", CallbackObj->UpdatedTasks[1], Q->GetTask("T_CollectDoobries"));
		TestEqual("Should have received correct task update callback", CallbackObj->UpdatedTasks[2], Q->GetTask("T_CollectDoobries"));
	}
	// Collecting one more should only raise 1 update, for the number increase only (no status change)
	Progression->ProgressTask("Q_Main1", "T_CollectDoobries", 1);
	TestEqual("Should be 2 extra task update callback for progress", CallbackObj->UpdatedTasks.Num(), 4);
	if (CallbackObj->UpdatedTasks.Num() == 4)
		TestEqual("Should have received correct task update callback", CallbackObj->UpdatedTasks[3], Q->GetTask("T_CollectDoobries"));

	TestEqual("Should be no failed tasks yet", CallbackObj->FailedTasks.Num(), 0);
	Progression->FailTask("Q_Main1", "T_CollectDoobries");
	TestEqual("Should be a task failed callback", CallbackObj->FailedTasks.Num(), 1);
	if (CallbackObj->FailedTasks.Num() > 0)
		TestEqual("Should have received correct task failure callback", CallbackObj->FailedTasks[0], Q->GetTask("T_CollectDoobries"));
	// this should NOT have triggered any Objective/Quest failure callbacks though (it's optional)
	TestEqual("Should be no failed objectives yet", CallbackObj->FailedObjectives.Num(), 0);
	TestEqual("Should be no failed quests yet", CallbackObj->FailedQuests.Num(), 0);
	
	// Complete another task that completes the objective
	TestTrue("Task complete should work", Progression->CompleteTask("Q_Main1", "T_DoTheThing"));
	TestEqual("Should be a objective complete callback", CallbackObj->CompletedObjectives.Num(), 1);
	if (CallbackObj->CompletedObjectives.Num() > 0)
		TestEqual("Should have received correct objective complete callback", CallbackObj->CompletedObjectives[0], Q->GetObjective("O1"));
	
	// Now fail the next objective
	Progression->FailTask("Q_Main1", "T_Something1");
	TestEqual("Should be a objective fail callback", CallbackObj->FailedObjectives.Num(), 1);
	if (CallbackObj->FailedObjectives.Num() > 0)
		TestEqual("Should have received correct objective failure callback", CallbackObj->FailedObjectives[0], Q->GetObjective("O2"));
	
	
	return true;
}
