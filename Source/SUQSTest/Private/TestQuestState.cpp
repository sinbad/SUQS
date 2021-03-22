#include "Misc/AutomationTest.h"
#include "Engine.h"
#include "Suqs.h"
#include "SuqsObjectiveState.h"
#include "SuqsProgression.h"
#include "SuqsTaskState.h"
#include "TestQuestData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestAcceptSimple, "SUQSTest.QuestAcceptSimple",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestAcceptSimple::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
        	USuqsProgression::MakeQuestDataTableFromJSON(SimpleMainQuestJson),
        	USuqsProgression::MakeQuestDataTableFromJSON(SimpleSideQuestJson)
        }
    );


	TestEqual("Main quest should be unavailable", Progression->GetQuestStatus("Q_Main1"), ESuqsQuestStatus::Unavailable);
	TestEqual("Side quest should be unavailable", Progression->GetQuestStatus("Q_Side1"), ESuqsQuestStatus::Unavailable);
	TestTrue("Should be able to accept main quest", Progression->AcceptQuest("Q_Main1"));
	TestEqual("Main quest should be incomplete", Progression->GetQuestStatus("Q_Main1"), ESuqsQuestStatus::Incomplete);
	TestTrue("Should be able to accept side quest", Progression->AcceptQuest("Q_Side1"));
	TestEqual("Side quest should be incomplete", Progression->GetQuestStatus("Q_Side1"), ESuqsQuestStatus::Incomplete);

	// This raises a warning, which would turn the test results yellow, so let's disable those
	LogSUQS.SetVerbosity(ELogVerbosity::NoLogging);
	TestFalse("Accepting Main quest again should do nothing", Progression->AcceptQuest("Q_Main1"));
	LogSUQS.SetVerbosity(ELogVerbosity::Verbose);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestAcceptFailedComplete, "SUQSTest.QuestAcceptFailedComplete",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestAcceptFailedComplete::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();

	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(SmallestPossibleQuestJson)
        }
    );

	// Test accepting failed
	TestTrue("Accept smallest quest", Progression->AcceptQuest("Q_Smol"));
	TestEqual("Smol quest should be incomplete", Progression->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Incomplete);
	Progression->FailQuest("Q_Smol");
	TestEqual("Smol quest should now be failed", Progression->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Failed);
	TestTrue("Accepting failed quest should succeeed (and reset)", Progression->AcceptQuest("Q_Smol"));
	TestEqual("Smol quest should be incomplete again", Progression->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Incomplete);
	// remove so we can go again
	Progression->RemoveQuest("Q_Smol");

	// Test accepting completed
	TestTrue("Accept smallest quest", Progression->AcceptQuest("Q_Smol"));
	TestEqual("Smol quest should be incomplete", Progression->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Incomplete);
	TestTrue("Smol task should complete OK", Progression->CompleteTask("Q_Smol", "T_Smol"));
	TestEqual("Smol quest should now be completed", Progression->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Completed);
	// Suppress warning logs
	LogSUQS.SetVerbosity(ELogVerbosity::NoLogging);
	TestFalse("Accepting completed quest should do nothing by default", Progression->AcceptQuest("Q_Smol"));
	LogSUQS.SetVerbosity(ELogVerbosity::Verbose);
	
	TestEqual("Smol quest should still be completed", Progression->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Completed);
	TestTrue("Accepting completed quest with reset option should reset", Progression->AcceptQuest("Q_Smol", false, true));
	TestEqual("Smol quest should be incomplete again", Progression->GetQuestStatus("Q_Smol"), ESuqsQuestStatus::Incomplete);

	
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestReset, "SUQSTest.QuestReset",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::ClientContext |
    EAutomationTestFlags::ProductFilter)

bool FTestQuestReset::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(SimpleMainQuestJson)
        }
    );

	TestTrue("Should be able to accept main quest", Progression->AcceptQuest("Q_Main1"));
	TestEqual("Main quest should be incomplete", Progression->GetQuestStatus("Q_Main1"), ESuqsQuestStatus::Incomplete);

	auto Q = Progression->GetQuest("Q_Main1");
	auto Obj = Q->GetCurrentObjective();
	TestNotNull("Current Objective should be valid", Obj);
	TestEqual("O1 objective should be current", Obj->GetIdentifier(), FName("O1"));
	TestEqual("O1 objective should be not started", Obj->GetStatus(), ESuqsObjectiveStatus::NotStarted);

	// Complete first task via top-level API
	TestTrue("Task should complete OK", Progression->CompleteTask("Q_Main1", "T_ReachThePlace"));
	TestEqual("Main quest should still be incomplete", Progression->GetQuestStatus("Q_Main1"), ESuqsQuestStatus::Incomplete);
	// Now let's do things via the object interface
	Obj = Q->GetCurrentObjective();
	TestNotNull("Current Objective should be valid", Obj);
	TestEqual("O1 objective should be current", Obj->GetIdentifier(), FName("O1"));
	TestEqual("O1 objective should be in progress", Obj->GetStatus(), ESuqsObjectiveStatus::InProgress);

	auto Tasks = Obj->GetTasks();
	TestEqual("Should be correct number of tasks", Tasks.Num(), 3);
	TestEqual("First task should be complete", Tasks[0]->GetStatus(), ESuqsTaskStatus::Completed);
	TestEqual("Second task should be not started", Tasks[1]->GetStatus(), ESuqsTaskStatus::NotStarted);

	// now complete the other (mandatory) task, but via the bottom level API
	// third task is optional so actually cannot be completed *after* the mandatory one
	TestTrue("Task should complete", Tasks[1]->Complete());
	// This should now complete one objective and set the next one to be active

	TestEqual("First task should be complete", Tasks[0]->GetStatus(), ESuqsTaskStatus::Completed);
	TestEqual("Second task should be complete", Tasks[1]->GetStatus(), ESuqsTaskStatus::Completed);
	TestEqual("O1 objective should be complete", Obj->GetStatus(), ESuqsObjectiveStatus::Completed);
	Obj = Q->GetCurrentObjective();
	TestNotNull("Current Objective should be valid", Obj);
	TestEqual("O2 objective should be current", Obj->GetIdentifier(), FName("O2"));
	TestEqual("O2 objective should be not started", Obj->GetStatus(), ESuqsObjectiveStatus::NotStarted);

	// We'll complete one task of the objective
	TestTrue("Task should complete", Obj->GetTasks()[0]->Complete());
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestOrderedTasks, "SUQSTest.QuestOrderedTasks",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::ClientContext |
    EAutomationTestFlags::ProductFilter)

bool FTestQuestOrderedTasks::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(OrderedTasksQuestJson)
        }
    );

	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_Ordered"));
	// Suppress the warnings this will raise
	LogSUQS.SetVerbosity(ELogVerbosity::NoLogging);
	TestFalse("Should not be allowed to complete task out of order", Progression->CompleteTask("Q_Ordered", "T_2"));
	TestFalse("Should not be allowed to complete task out of order", Progression->CompleteTask("Q_Ordered", "T_3"));
	TestFalse("Should not be allowed to complete a task on a later objective", Progression->CompleteTask("Q_Ordered", "T_11"));
	LogSUQS.SetVerbosity(ELogVerbosity::Verbose);
	
	TestTrue("Task 1 should complete OK", Progression->CompleteTask("Q_Ordered", "T_1"));
	LogSUQS.SetVerbosity(ELogVerbosity::NoLogging);
	TestFalse("Should not be allowed to complete task out of order", Progression->CompleteTask("Q_Ordered", "T_3"));
	LogSUQS.SetVerbosity(ELogVerbosity::Verbose);


	TestTrue("Task 2 should complete OK", Progression->CompleteTask("Q_Ordered", "T_2"));
	TestTrue("Completing an already completed previous task should pass but do nothing", Progression->CompleteTask("Q_Ordered", "T_1"));
	
	TestTrue("Task 3 should complete OK", Progression->CompleteTask("Q_Ordered", "T_3"));

	LogSUQS.SetVerbosity(ELogVerbosity::NoLogging);
	TestFalse("Objective 2 Task 2 is out of order still", Progression->CompleteTask("Q_Ordered", "T_12"));
	LogSUQS.SetVerbosity(ELogVerbosity::Verbose);

	TestTrue("Objective 2 Task 1 should complete OK now", Progression->CompleteTask("Q_Ordered", "T_11"));
	TestTrue("Objective 2 Task 2 should complete OK now", Progression->CompleteTask("Q_Ordered", "T_12"));

	TestTrue("Quest should be complete now", Progression->IsQuestCompleted("Q_Ordered"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestUnorderedTasks, "SUQSTest.QuestUnorderedTasks",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::ClientContext |
    EAutomationTestFlags::ProductFilter)

bool FTestQuestUnorderedTasks::RunTest(const FString& Parameters)
{

	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(UnorderedTasksQuestJson)
        }
    );

	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_Unordered"));
	TestTrue("Task 3 should complete out of order OK", Progression->CompleteTask("Q_Unordered", "T_3"));
	TestTrue("Task 1 should complete out of order OK", Progression->CompleteTask("Q_Unordered", "T_1"));
	TestTrue("Task 2 should complete out of order OK", Progression->CompleteTask("Q_Unordered", "T_2"));
	
	TestTrue("Quest should be complete now", Progression->IsQuestCompleted("Q_Unordered"));
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestAny2OfTasks, "SUQSTest.QuestAny2OfTasks",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::ClientContext |
    EAutomationTestFlags::ProductFilter)

bool FTestQuestAny2OfTasks::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(AnyOfTasksQuestJson)
        }
    );

	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_Any2Of"));
	TestTrue("Task 1 should complete OK", Progression->CompleteTask("Q_Any2Of", "T_1"));
	TestFalse("Quest should be incomplete after 1 task complete", Progression->IsQuestCompleted("Q_Any2Of"));
	TestTrue("Task 2 should complete OK", Progression->CompleteTask("Q_Any2Of", "T_2"));
	TestTrue("Quest should be complete after 2 tasks complete", Progression->IsQuestCompleted("Q_Any2Of"));

	Progression->ResetQuest("Q_Any2Of");
	TestFalse("Quest should be incomplete after reset", Progression->IsQuestCompleted("Q_Any2Of"));	

	TestTrue("Task 3 should complete OK", Progression->CompleteTask("Q_Any2Of", "T_3"));
	TestTrue("Task 1 should complete OK", Progression->CompleteTask("Q_Any2Of", "T_1"));
	TestTrue("Quest should be complete after any 2 tasks complete", Progression->IsQuestCompleted("Q_Any2Of"));

	Progression->ResetQuest("Q_Any2Of");
	TestFalse("Quest should be incomplete after reset", Progression->IsQuestCompleted("Q_Any2Of"));

	// Completing one mandatory and one optional task should not complete the any2of group
	TestTrue("Task 1 should complete OK", Progression->CompleteTask("Q_Any2Of", "T_1"));
	TestTrue("Optional task should complete OK", Progression->CompleteTask("Q_Any2Of", "T_Optional"));
	TestFalse("Quest should remain incomplete after 2 tasks if one is optional", Progression->IsQuestCompleted("Q_Any2Of"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestTargetNumber, "SUQSTest.QuestTargetNumber",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::ClientContext |
    EAutomationTestFlags::ProductFilter)

bool FTestTargetNumber::RunTest(const FString& Parameters)
{

	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(TargetNumberQuestJson)
        }
    );

	TestTrue("Accept quest should work", Progression->AcceptQuest("Q_TargetNumbers"));

	TestEqual("First task only has a target number of 1", Progression->ProgressTask("Q_TargetNumbers", "T_TargetOf1", 1), 0);
	TestTrue("First task should now be complete", Progression->IsTaskCompleted("Q_TargetNumbers", "T_TargetOf1"));

	TestEqual("Second task should be not started", Progression->GetTaskState("Q_TargetNumbers", "T_TargetOf3")->GetStatus(), ESuqsTaskStatus::NotStarted);
	TestEqual("Second task should have 2 left", Progression->ProgressTask("Q_TargetNumbers", "T_TargetOf3", 1), 2);
	
	TestEqual("Second task should be in progress", Progression->GetTaskState("Q_TargetNumbers", "T_TargetOf3")->GetStatus(), ESuqsTaskStatus::InProgress);
	TestEqual("Second task should have 0 left", Progression->ProgressTask("Q_TargetNumbers", "T_TargetOf3", 2), 0);
	TestTrue("Second task should be completed", Progression->IsTaskCompleted("Q_TargetNumbers", "T_TargetOf3"));

	TestEqual("Third task should have 4 left", Progression->ProgressTask("Q_TargetNumbers", "T_TargetOf6", 2), 4);
	TestEqual("Third task should have 3 left", Progression->ProgressTask("Q_TargetNumbers", "T_TargetOf6", 1), 3);
	// go backwards
	TestEqual("Third task should have 4 left after -1", Progression->ProgressTask("Q_TargetNumbers", "T_TargetOf6", -1), 4);
	// now go forward again, test clamping at max
	auto T3 = Progression->GetTaskState("Q_TargetNumbers", "T_TargetOf6");
	TestEqual("Third task should have 0 left", Progression->ProgressTask("Q_TargetNumbers", "T_TargetOf6", 12), 0);
	TestEqual("Third task should have clamped progress at max", T3->GetNumber(), 6);
	TestTrue("Third task should be completed", Progression->IsTaskCompleted("Q_TargetNumbers", "T_TargetOf6"));
	TestTrue("Quest should be complete now", Progression->IsQuestCompleted("Q_TargetNumbers"));
	
	// test going backwards from complete
	TestEqual("Third task should have 2 left after -2", Progression->ProgressTask("Q_TargetNumbers", "T_TargetOf6", -2), 2);
	TestEqual("Third task should be back in progress after negative", Progression->GetTaskState("Q_TargetNumbers", "T_TargetOf6")->GetStatus(), ESuqsTaskStatus::InProgress);
	TestFalse("Quest should be incomplete now", Progression->IsQuestCompleted("Q_TargetNumbers"));

	// test going backwards to negative, should clamp to 0
	TestEqual("Third task should have 6 left after -12", Progression->ProgressTask("Q_TargetNumbers", "T_TargetOf6", -12), 6);
	TestEqual("Third task should have clamped progress at 0", T3->GetNumber(), 0);
	TestEqual("Task should be not started again now", T3->GetStatus(), ESuqsTaskStatus::NotStarted);
	
	

	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestMultiObjective, "SUQSTest.QuestMultiObjective",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::ClientContext |
    EAutomationTestFlags::ProductFilter)

bool FTestMultiObjective::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(SimpleMainQuestJson)
        }
    );

	// I could access the task objects using the API but I want to test that the top-level interface is working, which
	// uses the nested objects anyway.
	FName TaskIDs[] = {
		"T_ReachThePlace",	// O1
		"T_DoTheThing",		// O1
		"T_CollectDoobries",// O1, optional
		"T_Something1",		// O2
		"T_Something2",		// O2
		"TOptionA",			// O3, either or
		"TOptionB"			// O3, either or
	};

	// Roll through the whole of a quest with multiple objectives
	TestTrue("Accept main quest OK", Progression->AcceptQuest("Q_Main1"));
	TestTrue("Quest should be incomplete", Progression->IsQuestIncomplete("Q_Main1"));
	TestTrue("Objective 1 should be incomplete", Progression->IsObjectiveIncomplete("Q_Main1", "O1"));
	TestTrue("Objective 2 should be incomplete", Progression->IsObjectiveIncomplete("Q_Main1", "O2"));
	TestTrue("Objective 3 should be incomplete", Progression->IsObjectiveIncomplete("Q_Main1", "O3"));
	for (auto& TaskID : TaskIDs)
	{
		TestTrue("Tasks should all be incomplete", Progression->IsTaskIncomplete("Q_Main1", TaskID));
	}

	// We won't complete the optional (index 2)
	for (int i = 0; i < 2; ++i)
	{
		TestTrue("Task should complete", Progression->CompleteTask("Q_Main1", TaskIDs[i]));
		TestTrue("Task should report completed", Progression->IsTaskCompleted("Q_Main1", TaskIDs[i]));
		if (i < 1)
			TestTrue("Objective 1 should be incomplete", Progression->IsObjectiveIncomplete("Q_Main1", "O1"));
		else
			TestTrue("Objective 1 should be completed even though optional wasn't", Progression->IsObjectiveCompleted("Q_Main1", "O1"));
	}
	TestTrue("Quest should be incomplete", Progression->IsQuestIncomplete("Q_Main1"));
	TestTrue("Objective 2 should be incomplete", Progression->IsObjectiveIncomplete("Q_Main1", "O2"));
	TestTrue("Objective 3 should be incomplete", Progression->IsObjectiveIncomplete("Q_Main1", "O3"));
	for (int i = 3; i < 5; ++i)
	{
		TestTrue("Task should complete", Progression->CompleteTask("Q_Main1", TaskIDs[i]));
		TestTrue("Task should report completed", Progression->IsTaskCompleted("Q_Main1", TaskIDs[i]));
		if (i < 4)
			TestTrue("Objective 2 should be incomplete", Progression->IsObjectiveIncomplete("Q_Main1", "O2"));
		else
			TestTrue("Objective 2 should be completed", Progression->IsObjectiveCompleted("Q_Main1", "O2"));
	}
	TestTrue("Quest should be incomplete", Progression->IsQuestIncomplete("Q_Main1"));
	TestTrue("Objective 3 should be incomplete", Progression->IsObjectiveIncomplete("Q_Main1", "O3"));

	// Last objective is either-or so complete only one
	TestTrue("Task should complete", Progression->CompleteTask("Q_Main1", TaskIDs[6]));
	TestTrue("Objective 3 should be completed", Progression->IsObjectiveCompleted("Q_Main1", "O3"));
	TestTrue("Quest should be complete", Progression->IsQuestCompleted("Q_Main1"));


	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestDescriptions, "SUQSTest.QuestDescriptions",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::ClientContext |
    EAutomationTestFlags::ProductFilter)

bool FTestDescriptions::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(SimpleMainQuestJson)
        }
    );

	TestTrue("Accept main quest OK", Progression->AcceptQuest("Q_Main1"));
	auto Q = Progression->GetQuest("Q_Main1");
	auto O1 = Q->GetObjective("O1");
	TestEqual("Quest description should be incomplete version", Q->GetDescription().ToString(), "This is the main quest");
	TestEqual("Objective description should be incomplete version", O1->GetDescription().ToString(), "This is the thing you do first");

	// complete the tasks (only 2 needed, 3rd is optional)
	TestTrue("Task 1 should complete OK", Q->CompleteTask("T_ReachThePlace"));
	TestEqual("Objective description should be incomplete version", O1->GetDescription().ToString(), "This is the thing you do first");
	TestTrue("Task 2 should complete OK", Q->CompleteTask("T_DoTheThing"));
	TestEqual("Objective description should have changed to complete version", O1->GetDescription().ToString(), "You did the thing you do first!");
	TestEqual("Quest description should still be incomplete version", Q->GetDescription().ToString(), "This is the main quest");

	// Complete everything en masse
	Q->Complete();
	TestEqual("Quest description should have changed to the complete version", Q->GetDescription().ToString(), "You did the main quest!");
	
	return true;
}