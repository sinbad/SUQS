#include "Misc/AutomationTest.h"
#include "Engine.h"
#include "SuqsProgression.h"
#include "TestQuestData.h"
#include "CallbackCatcher.h"
#include "SuqsTaskState.h"

UE_DISABLE_OPTIMIZATION_SHIP

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
		if (TestEqual("Event 0 should be task failed", CallbackObj->ProgressionEvents[0].EventType, ESuqsProgressionEventType::TaskFailed))
		{
			if (TestNotNull("Event 0 task should be not null", CallbackObj->ProgressionEvents[0].Task))
			{
				TestEqual("Event 0 should be correct task", CallbackObj->ProgressionEvents[0].Task->GetIdentifier(), FName("T_ReachThePlace"));
			}
		}
		if (TestEqual("Event 1 should be task removed", CallbackObj->ProgressionEvents[1].EventType, ESuqsProgressionEventType::TaskRemoved))
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
	CallbackObj->ProgressionEvents.Empty();
	
	Q->Reset();
	if (TestEqual("Should have got another accept callback on reset", CallbackObj->AcceptedQuests.Num(), 2))
		TestEqual("Should have received correct quest callback", CallbackObj->AcceptedQuests[1], Q);

	TestEqual("Should be no completed to start", CallbackObj->CompletedQuests.Num(), 0);

	// Don't test events here, they're messy on reset
	CallbackObj->ProgressionEvents.Empty();
	
	Q->Complete();
	if (TestEqual("Should get quest completion callback", CallbackObj->CompletedQuests.Num(), 1))
		TestEqual("Should have received correct quest callback", CallbackObj->CompletedQuests[0], Q);
	
	if (TestEqual("Should have received correct number of progress events", CallbackObj->ProgressionEvents.Num(), 27))
	{
		// Completing the quest completes ALL mandatory tasks. This accelerates the sequence all the way through
		if (TestEqual("Event 0 should be task complete", CallbackObj->ProgressionEvents[0].EventType, ESuqsProgressionEventType::TaskCompleted))
		{
			if (TestNotNull("Event 0 task should be not null", CallbackObj->ProgressionEvents[0].Task))
			{
				TestEqual("Event 0 should be correct task", CallbackObj->ProgressionEvents[0].Task->GetIdentifier(), FName("T_ReachThePlace"));
			}
		}
		if (TestEqual("Event 1 should be task removed", CallbackObj->ProgressionEvents[1].EventType, ESuqsProgressionEventType::TaskRemoved))
		{
			if (TestNotNull("Event 1 task should be not null", CallbackObj->ProgressionEvents[1].Task))
			{
				TestEqual("Event 1 should be correct task", CallbackObj->ProgressionEvents[1].Task->GetIdentifier(), FName("T_ReachThePlace"));
			}
		}
		if (TestEqual("Event 2 should be task added", CallbackObj->ProgressionEvents[2].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 2 task should be not null", CallbackObj->ProgressionEvents[2].Task))
			{
				TestEqual("Event 2 should be correct task", CallbackObj->ProgressionEvents[2].Task->GetIdentifier(), FName("T_DoTheThing"));
			}
		}
		if (TestEqual("Event 3 should be task complete", CallbackObj->ProgressionEvents[3].EventType, ESuqsProgressionEventType::TaskCompleted))
		{
			if (TestNotNull("Event 3 task should be not null", CallbackObj->ProgressionEvents[3].Task))
			{
				TestEqual("Event 3 should be correct task", CallbackObj->ProgressionEvents[3].Task->GetIdentifier(), FName("T_DoTheThing"));
			}
		}
		if (TestEqual("Event 4 should be task removed", CallbackObj->ProgressionEvents[4].EventType, ESuqsProgressionEventType::TaskRemoved))
		{
			if (TestNotNull("Event 4 task should be not null", CallbackObj->ProgressionEvents[4].Task))
			{
				TestEqual("Event 4 should be correct task", CallbackObj->ProgressionEvents[4].Task->GetIdentifier(), FName("T_DoTheThing"));
			}
		}
		// Completion should bubble up
		if (TestEqual("Event 5 should be objective complete", CallbackObj->ProgressionEvents[5].EventType, ESuqsProgressionEventType::ObjectiveCompleted))
		{
			if (TestNotNull("Event 5 objective should be not null", CallbackObj->ProgressionEvents[5].Objective))
			{
				TestEqual("Event 5 should be correct objective", CallbackObj->ProgressionEvents[5].Objective->GetIdentifier(), FName("O1"));
			}
		}
		// Optional task should be explicitly removed even though not complete
		if (TestEqual("Event 6 should be task removed", CallbackObj->ProgressionEvents[6].EventType, ESuqsProgressionEventType::TaskRemoved))
		{
			if (TestNotNull("Event 6 task should be not null", CallbackObj->ProgressionEvents[6].Task))
			{
				TestEqual("Event 6 should be correct task", CallbackObj->ProgressionEvents[6].Task->GetIdentifier(), FName("T_CollectDoobries"));
			}
		}
		// then it will switch to a new objective
		TestEqual("Event 7 should be current objective changed", CallbackObj->ProgressionEvents[7].EventType, ESuqsProgressionEventType::QuestCurrentObjectiveChanged);
		// then add the 2 non-sequential tasks from the next objective
		if (TestEqual("Event 8 should be task added", CallbackObj->ProgressionEvents[8].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 8 task should be not null", CallbackObj->ProgressionEvents[8].Task))
			{
				TestTrue("Event 8 task should be mandatory", CallbackObj->ProgressionEvents[8].Task->IsMandatory());
				TestEqual("Event 8 task should be correct", CallbackObj->ProgressionEvents[8].Task->GetIdentifier(), FName("T_Something1"));
			}
		}
		if (TestEqual("Event 9 should be task added", CallbackObj->ProgressionEvents[9].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 9 task should be not null", CallbackObj->ProgressionEvents[9].Task))
			{
				TestTrue("Event 9 task should be mandatory", CallbackObj->ProgressionEvents[9].Task->IsMandatory());
				TestEqual("Event 9 task should be correct", CallbackObj->ProgressionEvents[9].Task->GetIdentifier(), FName("T_Something2"));
			}
		}
		// Now both of those tasks will be completed!
		// BUT in this case neither will be removed, because tasks are NOT sequential (so they should both continue to be shown to the player until objective switches)
		if (TestEqual("Event 10 should be task complete", CallbackObj->ProgressionEvents[10].EventType, ESuqsProgressionEventType::TaskCompleted))
		{
			if (TestNotNull("Event 10 task should be not null", CallbackObj->ProgressionEvents[10].Task))
			{
				TestEqual("Event 10 should be correct task", CallbackObj->ProgressionEvents[10].Task->GetIdentifier(), FName("T_Something1"));
			}
		}
		if (TestEqual("Event 11 should be task complete", CallbackObj->ProgressionEvents[11].EventType, ESuqsProgressionEventType::TaskCompleted))
		{
			if (TestNotNull("Event 11 task should be not null", CallbackObj->ProgressionEvents[11].Task))
			{
				TestEqual("Event 11 should be correct task", CallbackObj->ProgressionEvents[11].Task->GetIdentifier(), FName("T_Something2"));
			}
		}
		// both non-sequentials are complete, so next event is objective complete
		if (TestEqual("Event 12 should be objective complete", CallbackObj->ProgressionEvents[12].EventType, ESuqsProgressionEventType::ObjectiveCompleted))
		{
			if (TestNotNull("Event 12 objective should be not null", CallbackObj->ProgressionEvents[12].Objective))
			{
				TestEqual("Event 12 should be correct objective", CallbackObj->ProgressionEvents[12].Objective->GetIdentifier(), FName("O2"));
			}
		}
		if (TestEqual("Event 13 should be task removed", CallbackObj->ProgressionEvents[13].EventType, ESuqsProgressionEventType::TaskRemoved))
		{
			if (TestNotNull("Event 13 task should be not null", CallbackObj->ProgressionEvents[13].Task))
			{
				TestEqual("Event 13 should be correct task", CallbackObj->ProgressionEvents[13].Task->GetIdentifier(), FName("T_Something1"));
			}
		}
		if (TestEqual("Event 14 should be task removed", CallbackObj->ProgressionEvents[14].EventType, ESuqsProgressionEventType::TaskRemoved))
		{
			if (TestNotNull("Event 14 task should be not null", CallbackObj->ProgressionEvents[14].Task))
			{
				TestEqual("Event 14 should be correct task", CallbackObj->ProgressionEvents[14].Task->GetIdentifier(), FName("T_Something2"));
			}
		}

		// then it will switch to our last objective
		TestEqual("Event 15 should be current objective changed", CallbackObj->ProgressionEvents[15].EventType, ESuqsProgressionEventType::QuestCurrentObjectiveChanged);
		// Last objective also has 2 non-sequential tasks
		if (TestEqual("Event 16 should be task added", CallbackObj->ProgressionEvents[16].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 16 task should be not null", CallbackObj->ProgressionEvents[16].Task))
			{
				TestTrue("Event 16 task should be mandatory", CallbackObj->ProgressionEvents[16].Task->IsMandatory());
				TestEqual("Event 16 task should be correct", CallbackObj->ProgressionEvents[16].Task->GetIdentifier(), FName("TOptionA"));
			}
		}
		if (TestEqual("Event 17 should be task added", CallbackObj->ProgressionEvents[17].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 17 task should be not null", CallbackObj->ProgressionEvents[17].Task))
			{
				TestTrue("Event 17 task should be mandatory", CallbackObj->ProgressionEvents[17].Task->IsMandatory());
				TestEqual("Event 17 task should be correct", CallbackObj->ProgressionEvents[17].Task->GetIdentifier(), FName("TOptionB"));
			}
		}
		// Now only 1 of these tasks will be completed, because the number of mandatory tasks is 1
		// Neither will be removed, because tasks are NOT sequential (so they should both continue to be shown to the player until objective switches)
		if (TestEqual("Event 18 should be task complete", CallbackObj->ProgressionEvents[18].EventType, ESuqsProgressionEventType::TaskCompleted))
		{
			if (TestNotNull("Event 18 task should be not null", CallbackObj->ProgressionEvents[18].Task))
			{
				TestEqual("Event 18 should be correct task", CallbackObj->ProgressionEvents[18].Task->GetIdentifier(), FName("TOptionA"));
			}
		}
		// The required number of tasks is complete, so next event is objective complete
		if (TestEqual("Event 19 should be objective complete", CallbackObj->ProgressionEvents[19].EventType, ESuqsProgressionEventType::ObjectiveCompleted))
		{
			if (TestNotNull("Event 19 objective should be not null", CallbackObj->ProgressionEvents[19].Objective))
			{
				TestEqual("Event 19 should be correct objective", CallbackObj->ProgressionEvents[19].Objective->GetIdentifier(), FName("O3"));
			}
		}
		// Quest complete
		if (TestEqual("Event 20 should be quest complete", CallbackObj->ProgressionEvents[20].EventType, ESuqsProgressionEventType::QuestCompleted))
		{
			TestEqual("Event 20 should be correct quest", CallbackObj->ProgressionEvents[20].Quest, Q);
		}
		if (TestEqual("Event 21 should be quest archived", CallbackObj->ProgressionEvents[21].EventType, ESuqsProgressionEventType::QuestArchived))
		{
			TestEqual("Event 21 should be correct quest", CallbackObj->ProgressionEvents[21].Quest, Q);
		}
		// This active quests changed came from the archiving
		TestEqual("Event 22 should be active quests changed", CallbackObj->ProgressionEvents[22].EventType, ESuqsProgressionEventType::ActiveQuestsChanged);
		// Once archived, this will trigger the acceptance of dependent quests!
		const auto Q2 = Progression->GetQuest("Q_Main2");
		TestNotNull("Quest 2 should have been auto-accepted", Q2);
		if (TestEqual("Event 23 should be quest accepted", CallbackObj->ProgressionEvents[23].EventType, ESuqsProgressionEventType::QuestAccepted))
		{
			TestEqual("Event 23 should be correct quest", CallbackObj->ProgressionEvents[23].Quest, Q2);
		}
		// This active quests changed came from the auto-accept
		TestEqual("Event 24 should be active quests changed", CallbackObj->ProgressionEvents[24].EventType, ESuqsProgressionEventType::ActiveQuestsChanged);
		
		if (TestEqual("Event 25 should be current objective", CallbackObj->ProgressionEvents[25].EventType, ESuqsProgressionEventType::QuestCurrentObjectiveChanged))
		{
			TestEqual("Event 25 should be correct quest", CallbackObj->ProgressionEvents[25].Quest, Q2);
		}
		if (TestEqual("Event 26 should be task added", CallbackObj->ProgressionEvents[26].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 26 task should be not null", CallbackObj->ProgressionEvents[26].Task))
			{
				TestTrue("Event 26 task should be mandatory", CallbackObj->ProgressionEvents[26].Task->IsMandatory());
				TestEqual("Event 26 task should be T_ReachThePlace", CallbackObj->ProgressionEvents[26].Task->GetIdentifier(), FName("T_DoTheThing"));
			}
		}
		
		
	}
	
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
	
	TestTrue("Task complete should work", Progression->CompleteTask("Q_Main1", "T_ReachThePlace"));
	if (TestEqual("Should be a task complete callback", CallbackObj->CompletedTasks.Num(), 1))
		TestEqual("Should have received correct task complete callback", CallbackObj->CompletedTasks[0], Q->GetTask("T_ReachThePlace"));

	if (TestEqual("Should have received correct number of progress events", CallbackObj->ProgressionEvents.Num(), 3))
	{
		if (TestEqual("Event 0 should be task completed", CallbackObj->ProgressionEvents[0].EventType, ESuqsProgressionEventType::TaskCompleted))
		{
			if (TestNotNull("Event 0 Task should not be null", CallbackObj->ProgressionEvents[0].Task))
				TestEqual("Event 0 should be correct task", CallbackObj->ProgressionEvents[0].Task->GetIdentifier(), FName("T_ReachThePlace"));
		}
		if (TestEqual("Event 1 should be task removed", CallbackObj->ProgressionEvents[1].EventType, ESuqsProgressionEventType::TaskRemoved))
		{
			if (TestNotNull("Event 1 Task should not be null", CallbackObj->ProgressionEvents[1].Task))
				TestEqual("Event 1 should be correct task", CallbackObj->ProgressionEvents[1].Task->GetIdentifier(), FName("T_ReachThePlace"));
		}
		if (TestEqual("Event 2 should be task added", CallbackObj->ProgressionEvents[2].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 2 Task should not be null", CallbackObj->ProgressionEvents[2].Task))
				TestEqual("Event 2 should be correct task", CallbackObj->ProgressionEvents[2].Task->GetIdentifier(), FName("T_DoTheThing"));
		}
	}
	CallbackObj->ProgressionEvents.Empty();
	
	Progression->ProgressTask("Q_Main1", "T_CollectDoobries", 1);
	if (TestEqual("Should be a task update callback for progress", CallbackObj->UpdatedTasks.Num(), 1))
	{
		TestEqual("Should have received correct task update callback", CallbackObj->UpdatedTasks[0], Q->GetTask("T_CollectDoobries"));
	}

	if (TestEqual("Should have received correct number of progress events", CallbackObj->ProgressionEvents.Num(), 1))
	{
		if (TestEqual("Event 0 should be task updated", CallbackObj->ProgressionEvents[0].EventType, ESuqsProgressionEventType::TaskUpdated))
		{
			if (TestNotNull("Event 0 Task should not be null", CallbackObj->ProgressionEvents[0].Task))
				TestEqual("Event 0 should be correct task", CallbackObj->ProgressionEvents[0].Task->GetIdentifier(), FName("T_CollectDoobries"));
		}
	}
	CallbackObj->ProgressionEvents.Empty();
	
	// Collecting one more should only raise 1 update, for the number increase only (no status change)
	Progression->ProgressTask("Q_Main1", "T_CollectDoobries", 1);
	if (TestEqual("Should be 1 extra task update callback for progress", CallbackObj->UpdatedTasks.Num(), 2))
		TestEqual("Should have received correct task update callback", CallbackObj->UpdatedTasks[1], Q->GetTask("T_CollectDoobries"));

	if (TestEqual("Should have received correct number of progress events", CallbackObj->ProgressionEvents.Num(), 1))
	{
		if (TestEqual("Event 0 should be task updated", CallbackObj->ProgressionEvents[0].EventType, ESuqsProgressionEventType::TaskUpdated))
		{
			if (TestNotNull("Event 0 Task should not be null", CallbackObj->ProgressionEvents[0].Task))
				TestEqual("Event 0 should be correct task", CallbackObj->ProgressionEvents[0].Task->GetIdentifier(), FName("T_CollectDoobries"));
		}
	}
	CallbackObj->ProgressionEvents.Empty();


	TestEqual("Should be no failed tasks yet", CallbackObj->FailedTasks.Num(), 0);
	Progression->FailTask("Q_Main1", "T_CollectDoobries");
	TestEqual("Should be a task failed callback", CallbackObj->FailedTasks.Num(), 1);
	if (CallbackObj->FailedTasks.Num() > 0)
		TestEqual("Should have received correct task failure callback", CallbackObj->FailedTasks[0], Q->GetTask("T_CollectDoobries"));
	// this should NOT have triggered any Objective/Quest failure callbacks though (it's optional)
	TestEqual("Should be no failed objectives yet", CallbackObj->FailedObjectives.Num(), 0);
	TestEqual("Should be no failed quests yet", CallbackObj->FailedQuests.Num(), 0);

	if (TestEqual("Should have received correct number of progress events", CallbackObj->ProgressionEvents.Num(), 1))
	{
		if (TestEqual("Event 0 should be task failed", CallbackObj->ProgressionEvents[0].EventType, ESuqsProgressionEventType::TaskFailed))
		{
			if (TestNotNull("Event 0 Task should not be null", CallbackObj->ProgressionEvents[0].Task))
				TestEqual("Event 0 should be correct task", CallbackObj->ProgressionEvents[0].Task->GetIdentifier(), FName("T_CollectDoobries"));
		}

		// This was an optional task so shouldn't have made any objective / quest change
	}
	CallbackObj->ProgressionEvents.Empty();

	// Complete another task that completes the objective
	TestTrue("Task complete should work", Progression->CompleteTask("Q_Main1", "T_DoTheThing"));
	if (TestEqual("Should be a objective complete callback", CallbackObj->CompletedObjectives.Num(), 1))
		TestEqual("Should have received correct objective complete callback", CallbackObj->CompletedObjectives[0], Q->GetObjective("O1"));

	if (TestEqual("Should have received correct number of progress events", CallbackObj->ProgressionEvents.Num(), 7))
	{
		if (TestEqual("Event 0 should be task complete", CallbackObj->ProgressionEvents[0].EventType, ESuqsProgressionEventType::TaskCompleted))
		{
			if (TestNotNull("Event 0 task should be not null", CallbackObj->ProgressionEvents[0].Task))
			{
				TestEqual("Event 0 should be correct task", CallbackObj->ProgressionEvents[0].Task->GetIdentifier(), FName("T_DoTheThing"));
			}
		}
		if (TestEqual("Event 1 should be task removed", CallbackObj->ProgressionEvents[1].EventType, ESuqsProgressionEventType::TaskRemoved))
		{
			if (TestNotNull("Event 1 task should be not null", CallbackObj->ProgressionEvents[1].Task))
			{
				TestEqual("Event 1 should be correct task", CallbackObj->ProgressionEvents[1].Task->GetIdentifier(), FName("T_DoTheThing"));
			}
		}
		// Completion should bubble up
		if (TestEqual("Event 2 should be objective complete", CallbackObj->ProgressionEvents[2].EventType, ESuqsProgressionEventType::ObjectiveCompleted))
		{
			if (TestNotNull("Event 2 objective should be not null", CallbackObj->ProgressionEvents[2].Objective))
			{
				TestEqual("Event 2 should be correct objective", CallbackObj->ProgressionEvents[2].Objective->GetIdentifier(), FName("O1"));
			}
		}
		// Objective completion should remove the optional task
		if (TestEqual("Event 3 should be task removed", CallbackObj->ProgressionEvents[3].EventType, ESuqsProgressionEventType::TaskRemoved))
		{
			if (TestNotNull("Event 3 task should be not null", CallbackObj->ProgressionEvents[3].Task))
			{
				TestEqual("Event 3 should be correct task", CallbackObj->ProgressionEvents[3].Task->GetIdentifier(), FName("T_CollectDoobries"));
			}
		}
		// then it will switch to a new objective
		TestEqual("Event 4 should be current objective changed", CallbackObj->ProgressionEvents[4].EventType, ESuqsProgressionEventType::QuestCurrentObjectiveChanged);
		// then add the 2 non-sequential tasks from the next objective
		if (TestEqual("Event 5 should be task added", CallbackObj->ProgressionEvents[5].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 5 task should be not null", CallbackObj->ProgressionEvents[5].Task))
			{
				TestTrue("Event 5 task should be mandatory", CallbackObj->ProgressionEvents[5].Task->IsMandatory());
				TestEqual("Event 5 task should be correct", CallbackObj->ProgressionEvents[5].Task->GetIdentifier(), FName("T_Something1"));
			}
		}
		if (TestEqual("Event 6 should be task added", CallbackObj->ProgressionEvents[6].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 6 task should be not null", CallbackObj->ProgressionEvents[6].Task))
			{
				TestTrue("Event 6 task should be mandatory", CallbackObj->ProgressionEvents[6].Task->IsMandatory());
				TestEqual("Event 6 task should be correct", CallbackObj->ProgressionEvents[6].Task->GetIdentifier(), FName("T_Something2"));
			}
		}
	}
	CallbackObj->ProgressionEvents.Empty();
	
	// Now fail the next objective
	Progression->FailTask("Q_Main1", "T_Something1");
	TestEqual("Should be a objective fail callback", CallbackObj->FailedObjectives.Num(), 1);
	if (CallbackObj->FailedObjectives.Num() > 0)
		TestEqual("Should have received correct objective failure callback", CallbackObj->FailedObjectives[0], Q->GetObjective("O2"));
	
	if (TestEqual("Should have received correct number of progress events", CallbackObj->ProgressionEvents.Num(), 5))
	{
		if (TestEqual("Event 0 should be task failed", CallbackObj->ProgressionEvents[0].EventType, ESuqsProgressionEventType::TaskFailed))
		{
			if (TestNotNull("Event 0 task should be not null", CallbackObj->ProgressionEvents[0].Task))
			{
				TestEqual("Event 0 should be correct task", CallbackObj->ProgressionEvents[0].Task->GetIdentifier(), FName("T_Something1"));
			}
		}
		if (TestEqual("Event 1 should be objective failed", CallbackObj->ProgressionEvents[1].EventType, ESuqsProgressionEventType::ObjectiveFailed))
		{
			if (TestNotNull("Event 1 task should be not null", CallbackObj->ProgressionEvents[1].Objective))
			{
				TestEqual("Event 1 should be correct task", CallbackObj->ProgressionEvents[1].Objective->GetIdentifier(), FName("O2"));
			}
		}
		if (TestEqual("Event 2 should be quest failed", CallbackObj->ProgressionEvents[2].EventType, ESuqsProgressionEventType::QuestFailed))
		{
			TestEqual("Event 2 should be correct quest", CallbackObj->ProgressionEvents[2].Quest, Q);
		}
		// Failed quests should be archived, then list updated
		if (TestEqual("Event 3 should be quest archived", CallbackObj->ProgressionEvents[3].EventType, ESuqsProgressionEventType::QuestArchived))
		{
			if (TestNotNull("Event 3 quest should be not null", CallbackObj->ProgressionEvents[3].Quest))
			{
				TestEqual("Event 3 should be correct quest", CallbackObj->ProgressionEvents[3].Quest, Q);
			}
		}
		TestEqual("Event 4 should be active quests changed", CallbackObj->ProgressionEvents[4].EventType, ESuqsProgressionEventType::ActiveQuestsChanged);
	}
	CallbackObj->ProgressionEvents.Empty();

	// Now let's go back and complete the task instead
	// Don't check events for this, this condition is checked elsewhere
	Progression->ResetTask("Q_Main1", "T_Something1");
	Progression->CompleteTask("Q_Main1", "T_Something2");
	Progression->CompleteTask("Q_Main1", "T_Something1");
	CallbackObj->ProgressionEvents.Empty();

	// Now test the next objective, which is either / or task
	TestEqual("Objective should be correct", Progression->GetCurrentObjective("Q_Main1")->GetIdentifier(), FName("O3"));
	Progression->CompleteTask("Q_Main1","TOptionB");
	if (TestEqual("Should have received correct number of progress events", CallbackObj->ProgressionEvents.Num(), 9))
	{
		if (TestEqual("Event 0 should be task completed", CallbackObj->ProgressionEvents[0].EventType, ESuqsProgressionEventType::TaskCompleted))
		{
			if (TestNotNull("Event 0 task should be not null", CallbackObj->ProgressionEvents[0].Task))
			{
				TestEqual("Event 0 should be correct task", CallbackObj->ProgressionEvents[0].Task->GetIdentifier(), FName("TOptionB"));
			}
		}
		if (TestEqual("Event 1 should be objective completed", CallbackObj->ProgressionEvents[1].EventType, ESuqsProgressionEventType::ObjectiveCompleted))
		{
			if (TestNotNull("Event 1 objective should be not null", CallbackObj->ProgressionEvents[1].Objective))
			{
				TestEqual("Event 1 should be correct objective", CallbackObj->ProgressionEvents[1].Objective->GetIdentifier(), FName("O3"));
			}
		}
		if (TestEqual("Event 2 should be quest completed", CallbackObj->ProgressionEvents[2].EventType, ESuqsProgressionEventType::QuestCompleted))
		{
			if (TestNotNull("Event 2 quest should be not null", CallbackObj->ProgressionEvents[2].Quest))
			{
				TestEqual("Event 2 should be correct quest", CallbackObj->ProgressionEvents[2].Quest->GetIdentifier(), FName("Q_Main1"));
			}
		}
		if (TestEqual("Event 3 should be quest archived", CallbackObj->ProgressionEvents[3].EventType, ESuqsProgressionEventType::QuestArchived))
		{
			if (TestNotNull("Event 3 quest should be not null", CallbackObj->ProgressionEvents[3].Quest))
			{
				TestEqual("Event 3 should be correct quest", CallbackObj->ProgressionEvents[3].Quest->GetIdentifier(), FName("Q_Main1"));
			}
		}
		// Should proceed to next quest
		TestEqual("Event 4 should be quests changed", CallbackObj->ProgressionEvents[4].EventType, ESuqsProgressionEventType::ActiveQuestsChanged);
		if (TestEqual("Event 5 should be quest accepted", CallbackObj->ProgressionEvents[5].EventType, ESuqsProgressionEventType::QuestAccepted))
		{
			if (TestNotNull("Event 5 quest should be not null", CallbackObj->ProgressionEvents[5].Quest))
			{
				TestEqual("Event 5 should be correct quest", CallbackObj->ProgressionEvents[5].Quest->GetIdentifier(), FName("Q_Main2"));
			}
		}
		TestEqual("Event 6 should be quests changed", CallbackObj->ProgressionEvents[6].EventType, ESuqsProgressionEventType::ActiveQuestsChanged);
		if (TestEqual("Event 7 should be quest objective changed", CallbackObj->ProgressionEvents[7].EventType, ESuqsProgressionEventType::QuestCurrentObjectiveChanged))
		{
			if (TestNotNull("Event 7 quest should be not null", CallbackObj->ProgressionEvents[7].Quest))
			{
				TestEqual("Event 7 should be correct quest", CallbackObj->ProgressionEvents[7].Quest->GetIdentifier(), FName("Q_Main2"));
			}
		}
		if (TestEqual("Event 8 should be quest task added", CallbackObj->ProgressionEvents[8].EventType, ESuqsProgressionEventType::TaskAdded))
		{
			if (TestNotNull("Event 8 task should be not null", CallbackObj->ProgressionEvents[8].Task))
			{
				TestEqual("Event 8 should be correct task", CallbackObj->ProgressionEvents[8].Task->GetIdentifier(), FName("T_DoTheThing"));
			}
		}
		
	}
	
	return true;
}

UE_ENABLE_OPTIMIZATION_SHIP