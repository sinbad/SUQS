#include "Misc/AutomationTest.h"
#include "Engine.h"
#include "SuqsProgression.h"
#include "SuqsTaskState.h"
#include "TestQuestData.h"



IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestTimeLimitSimple, "SUQSTest.QuestTimeLimitSimple",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestTimeLimitSimple::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->QuestDataTables.Add(USuqsProgression::MakeQuestDataTableFromJSON(TimeLimitQuestJson));

	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_TimeLimits"));
	TestTrue("Quest should be incomplete", Progression->IsQuestIncomplete("Q_TimeLimits"));

	// Manually tick
	Progression->Tick(10);
	TestFalse("Quest should not be failed", Progression->IsQuestFailed("Q_TimeLimits"));
	// Ticking past where later objectives would fail, but should not fail because only first objective should tick
	// and that task has a longer timeout
	Progression->Tick(60); // total 70
	TestFalse("Quest should not be failed", Progression->IsQuestFailed("Q_TimeLimits"));
	TestFalse("Inactive tasks should not be failed", Progression->IsTaskFailed("Q_TimeLimits", "T_SecondTimeLimited"));
	// Now tick beyond first task time limit
	Progression->Tick(30.1); // total 100.1
	TestTrue("Quest should have failed due to timeout", Progression->IsQuestFailed("Q_TimeLimits"));
	TestTrue("Objective should have failed due to timeout", Progression->IsObjectiveFailed("Q_TimeLimits", "O_Single"));
	TestTrue("Task should have failed due to timeout", Progression->IsTaskFailed("Q_TimeLimits", "T_Single"));
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestTimeLimitNotFirstTask, "SUQSTest.QuestTimeLimitNotFirstTask",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestTimeLimitNotFirstTask::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->QuestDataTables.Add(USuqsProgression::MakeQuestDataTableFromJSON(TimeLimitQuestJson));

	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_TimeLimits"));
	auto T = Progression->GetTaskState("Q_TimeLimits", "T_SecondTimeLimited");
	float OrigTimeLeft = T->GetTimeRemaining();
	// Tick a little and confirm it doesn't change the timeout
	Progression->Tick(10);
	TestEqual("Ticking while objective inactive should not have changed second objective", T->GetTimeRemaining(), OrigTimeLeft);
	// Complete first objective so we can start on the next
	TestTrue("Completing task should have worked", Progression->CompleteTask("Q_TimeLimits", "T_Single"));
	TestEqual("First task of second objective should be current", Progression->GetNextMandatoryTask("Q_TimeLimits")->GetIdentifier(), FName("T_NonTimeLimited"));
	// this task isn't time limited, and objective is sequential, so again ticking should not affect the time limited task
	Progression->Tick(10);
	TestEqual("Ticking while time limited task isn't next shouldn't reduce time left", T->GetTimeRemaining(), OrigTimeLeft);
	// Complete the non-timelimited task
	TestTrue("Completing task should have worked", Progression->CompleteTask("Q_TimeLimits", "T_NonTimeLimited"));
	// OK time limited task should be active now, and should reduce
	Progression->Tick(10);
	TestTrue("Second task should be reducing time limit now", T->GetTimeRemaining() < (OrigTimeLeft - 9.9));
	TestFalse("Quest should not be failed yet", Progression->IsQuestFailed("Q_TimeLimits"));
	Progression->Tick(50); // Total 60 now
	TestEqual("Second task should bhave run out of time", T->GetTimeRemaining(), 0.f);
	TestTrue("Quest should have failed now", Progression->IsQuestFailed("Q_TimeLimits"));

	// Reset so we can test having completed within time limit
	T->Reset();
	TestEqual("Time limit should be back", T->GetTimeRemaining(), OrigTimeLeft);
	TestFalse("Quest should not be failed anymore", Progression->IsQuestFailed("Q_TimeLimits"));
	Progression->Tick(15); // Tick a little within limit
	float TimeRemainingAtCompletion = T->GetTimeRemaining();
	TestEqual("Time remaining should be correct", TimeRemainingAtCompletion, T->GetTimeLimit() - 15);
	// Complete the tiem limited task
	T->Complete();
	// Same objective should still be active, but now moved on to last task
	// now tick again, and make sure this didn't affect the completed task
	Progression->Tick(50);
	TestEqual("Time remaining should not have changed", T->GetTimeRemaining(), TimeRemainingAtCompletion);

	return true;
}


const FString TimeLimitMultipleTasksQuestJson = R"RAWJSON([
    {
        "Identifier": "Q_TimeLimitsMulti",
        "bPlayerVisible": true,
        "Title": "NSLOCTEXT(\"[TestQuests]\", \"TimeLimitQuestTitle\", \"Quest With Time Limit\")",
        "DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"TimeLimitQuestDesc\", \"A quest with a time limit\")",
        "DescriptionWhenCompleted": "",
        "AutoAccept": false,
        "PrerequisiteQuests": [],
        "PrerequisiteQuestFailures": [],
        "Objectives": [
			{
				"Identifier": "O_MultipleTimeLimitsNonSequential",
			    "Title": "NSLOCTEXT(\"[TestQuests]\", \"DummyTitle\", \"Ignore this for test\")",
			    "DescriptionWhenActive": "",
			    "DescriptionWhenCompleted": "",
			    "bSequentialTasks": false,
			    "Branch": "None",
			    "Tasks": [
			        {
			            "Identifier": "T_NonSequential1",
			            "Title": "NSLOCTEXT(\"[TestQuests]\", \"TNonSeqNonMandTitle\", \"Non-sequential time limited task, not mandatory\")",
			            "bMandatory": true,
			            "TargetNumber": 1,
			            "TimeLimit": 50
			        },
			        {
			            "Identifier": "T_NonSequential2",
			            "Title": "NSLOCTEXT(\"[TestQuests]\", \"TNonSeqNonMandTitle\", \"Non-sequential time limited task, not mandatory\")",
			            "bMandatory": false,
			            "TargetNumber": 1,
			            "TimeLimit": 20
			        },
			        {
			            "Identifier": "T_NonSequential3",
			            "Title": "NSLOCTEXT(\"[TestQuests]\", \"TNonSeqMandTitle\", \"Non-sequential time limited task, the only mandatory\")",
			            "bMandatory": true,
			            "TargetNumber": 1,
			            "TimeLimit": 30
			        }
			    ]
			}
		]
    },
])RAWJSON";

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestTimeLimitMultipleSimultaneousTasks, "SUQSTest.QuestTimeLimitMultipleSimultaneousTasks",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestTimeLimitMultipleSimultaneousTasks::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->QuestDataTables.Add(USuqsProgression::MakeQuestDataTableFromJSON(TimeLimitMultipleTasksQuestJson));

	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_TimeLimitsMulti"));
	TestTrue("Quest should be incomplete", Progression->IsQuestIncomplete("Q_TimeLimitsMulti"));
	auto T1 = Progression->GetTaskState("Q_TimeLimitsMulti", "T_NonSequential1");
	auto T2 = Progression->GetTaskState("Q_TimeLimitsMulti", "T_NonSequential2");
	auto T3 = Progression->GetTaskState("Q_TimeLimitsMulti", "T_NonSequential3");

	Progression->Tick(10);
	TestEqual("T1 should have ticked", T1->GetTimeRemaining(), T1->GetTimeLimit() - 10);
	TestEqual("T2 should have ticked", T2->GetTimeRemaining(), T2->GetTimeLimit() - 10);
	TestEqual("T3 should have ticked", T3->GetTimeRemaining(), T3->GetTimeLimit() - 10);
	Progression->Tick(11);
	TestEqual("T1 should have ticked", T1->GetTimeRemaining(), T1->GetTimeLimit() - 21);
	TestEqual("T3 should have ticked", T3->GetTimeRemaining(), T3->GetTimeLimit() - 21);
	TestTrue("T2 should have failed", T2->IsFailed());
	// Task 2 is non-mandatory though
	TestFalse("Quest should not have failed", Progression->IsQuestFailed("Q_TimeLimitsMulti"));
	// Tick enough that Task 3 fails which is mandatory (even if not sequential)
	Progression->Tick(10);
	TestTrue("T3 should have failed", T3->IsFailed());
	TestTrue("Quest should have failed", Progression->IsQuestFailed("Q_TimeLimitsMulti"));

	return true;
}