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

	TestEqual("Should be no events to start", CallbackObj->ProgressionEvents.Num(), 0);

	TestEqual("Should be no accepted to start", CallbackObj->AcceptedQuests.Num(), 0);
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_Main1"));
	const auto Q = Progression->GetQuest("Q_Main1");
	if (TestEqual("Should have got one accept callback", CallbackObj->AcceptedQuests.Num(), 1))
		TestEqual("Should have received correct quest callback", CallbackObj->AcceptedQuests[0], Q);


	// Should have received these events
	// 1. Accept quest
	// 2. Objective changed
	// 3. Task added (mandatory, sequential)
	// 4. Task added (optional)
	if (TestEqual("Should have received correct number of progress events", CallbackObj->ProgressionEvents.Num(), 5))
	{
		if (TestEqual("Event 0 should be quest accepted", CallbackObj->ProgressionEvents[0].EventType, ESuqsProgressionEventType::QuestAccepted))
		{
			TestEqual("Event 0 should be correct quest", CallbackObj->ProgressionEvents[0].Quest, Q);
		}
		TestEqual("Event 1 should be active quests changed", CallbackObj->ProgressionEvents[1].EventType, ESuqsProgressionEventType::ActiveQuestsChanged);
		if (TestEqual("Event 2 should be current objective", CallbackObj->ProgressionEvents[2].EventType, ESuqsProgressionEventType::QuestCurrentObjectiveChanged))
		{
			TestEqual("Event 2 should be correct quest", CallbackObj->ProgressionEvents[2].Quest, Q);
		}
		if (TestEqual("Event 3 should be task added", CallbackObj->ProgressionEvents[3].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 3 task should be not null", CallbackObj->ProgressionEvents[3].Task))
			{
				TestTrue("Event 3 task should be mandatory", CallbackObj->ProgressionEvents[3].Task->IsMandatory());
				TestEqual("Event 3 task should be T_ReachThePlace", CallbackObj->ProgressionEvents[3].Task->GetIdentifier(), FName("T_ReachThePlace"));
			}
		}
		if (TestEqual("Event 4 should be task added", CallbackObj->ProgressionEvents[4].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 4 task should be not null", CallbackObj->ProgressionEvents[4].Task))
			{
				TestFalse("Event 4 task should not be mandatory", CallbackObj->ProgressionEvents[4].Task->IsMandatory());
				TestEqual("Event 4 task should be T_CollectDoobries", CallbackObj->ProgressionEvents[4].Task->GetIdentifier(), FName("T_CollectDoobries"));
			}
		}
	}

	CallbackObj->ProgressionEvents.Empty();

	TestEqual("Should be no failed to start", CallbackObj->FailedQuests.Num(), 0);
	Progression->GetNextMandatoryTask("Q_Main1")->Fail();
	if (TestEqual("Should be a failed callback", CallbackObj->FailedQuests.Num(), 1))
		TestEqual("Should have received correct quest callback", CallbackObj->FailedQuests[0], Q);
	
	if (TestEqual("Should have received correct number of progress events", CallbackObj->ProgressionEvents.Num(), 6))
	{
		// Failing quest fails the active task
		if (TestEqual("Event 0 should be task updated", CallbackObj->ProgressionEvents[0].EventType, ESuqsProgressionEventType::TaskUpdated))
		{
			if (TestNotNull("Event 0 task should be not null", CallbackObj->ProgressionEvents[0].Task))
			{
				TestEqual("Event 0 should be correct task", CallbackObj->ProgressionEvents[0].Task->GetIdentifier(), FName("T_ReachThePlace"));
			}
		}
		if (TestEqual("Event 1 should be task failed", CallbackObj->ProgressionEvents[1].EventType, ESuqsProgressionEventType::TaskFailed))
		{
			if (TestNotNull("Event 1 task should be not null", CallbackObj->ProgressionEvents[1].Task))
			{
				TestEqual("Event 1 should be correct task", CallbackObj->ProgressionEvents[1].Task->GetIdentifier(), FName("T_ReachThePlace"));
			}
		}
		// Failure should bubble up
		if (TestEqual("Event 2 should be objective failed", CallbackObj->ProgressionEvents[2].EventType, ESuqsProgressionEventType::ObjectiveFailed))
		{
			if (TestNotNull("Event 2 objective should be not null", CallbackObj->ProgressionEvents[2].Objective))
			{
				TestEqual("Event 2 should be correct objective", CallbackObj->ProgressionEvents[2].Objective->GetIdentifier(), FName("O1"));
			}
		}
		if (TestEqual("Event 3 should be quest failed", CallbackObj->ProgressionEvents[3].EventType, ESuqsProgressionEventType::QuestFailed))
		{
			if (TestNotNull("Event 3 quest should be not null", CallbackObj->ProgressionEvents[3].Quest))
			{
				TestEqual("Event 3 should be correct quest", CallbackObj->ProgressionEvents[3].Quest, Q);
			}
		}
		// Failed quests should be archived, then list updated
		if (TestEqual("Event 4 should be quest archived", CallbackObj->ProgressionEvents[4].EventType, ESuqsProgressionEventType::QuestArchived))
		{
			if (TestNotNull("Event 4 quest should be not null", CallbackObj->ProgressionEvents[4].Quest))
			{
				TestEqual("Event 4 should be correct quest", CallbackObj->ProgressionEvents[4].Quest, Q);
			}
		}
		TestEqual("Event 5 should be active quests changed", CallbackObj->ProgressionEvents[5].EventType, ESuqsProgressionEventType::ActiveQuestsChanged);
		
		
	}
	
	Q->Reset();
	if (TestEqual("Should have got another accept callback on reset", CallbackObj->AcceptedQuests.Num(), 2))
		TestEqual("Should have received correct quest callback", CallbackObj->AcceptedQuests[1], Q);

	TestEqual("Should be no completed to start", CallbackObj->CompletedQuests.Num(), 0);
	Q->Complete();
	if (TestEqual("Should get quest completion callback", CallbackObj->CompletedQuests.Num(), 1))
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
