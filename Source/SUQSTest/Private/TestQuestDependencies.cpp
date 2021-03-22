#include "Misc/AutomationTest.h"
#include "Engine.h"
#include "SuqsProgression.h"


const FString TriggerQuestsJson = R"RAWJSON([
	{
		"Identifier": "Q_TriggerQuest1",
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"TriggerQuestTitle\", \"Quest which triggers others\")",
		"Objectives": [
			{
				"Identifier": "O1",
				"Tasks": [
					{
						"Identifier": "T1",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"TSingleDesc\", \"Single task\")"
					}
				]
			}
		]
	},
	{
		"Identifier": "Q_TriggerQuest2",
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"TriggerQuestTitle\", \"Quest which triggers others\")",
		"Objectives": [
			{
				"Identifier": "O1",
				"Tasks": [
					{
						"Identifier": "T1",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"TSingleDesc\", \"Single task\")"
					}
				]
			}
		]
	}
])RAWJSON";

const FString SuccessDependentQuestJson = R"RAWJSON([
	{
		"Identifier": "Q_SuccessDeps",
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"SuccessDepQuestTitle\", \"SuccessDep Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"SuccessDepQuestDesc\", \"The smallest possible quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": true,
		"PrerequisiteQuests": ["Q_TriggerQuest1", "Q_TriggerQuest2"],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"SuccessDepObjTitle\", \"Small Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"SuccessDepObjDesc\", \"This is super easy\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_SuccessDep",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"SuccessDepQuestTaskTitle\", \"Easiest possible thing\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
])RAWJSON";

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestSuccessDependencies, "SUQSTest.QuestSuccessDependencies",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestSuccessDependencies::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
		TArray<UDataTable*> {
			USuqsProgression::MakeQuestDataTableFromJSON(TriggerQuestsJson),
			USuqsProgression::MakeQuestDataTableFromJSON(SuccessDependentQuestJson)
		}
	);

	// Accept the 2 trigger quests
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_TriggerQuest1"));
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_TriggerQuest2"));

	TestFalse("Dependent quest must not be accepted yet", Progression->IsQuestAccepted("Q_SuccessDeps"));
	// complete one trigger quest
	Progression->CompleteQuest("Q_TriggerQuest1");
	TestFalse("Dependent quest must not be accepted yet", Progression->IsQuestAccepted("Q_SuccessDeps"));
	// FAIL the other one
	Progression->FailQuest("Q_TriggerQuest2");
	TestFalse("Dependent quest must not be accepted yet", Progression->IsQuestAccepted("Q_SuccessDeps"));
	// now reset second
	Progression->ResetQuest("Q_TriggerQuest2");
	TestFalse("Dependent quest must not be accepted yet", Progression->IsQuestAccepted("Q_SuccessDeps"));
	// Complete the second, should trigger now
	Progression->CompleteQuest("Q_TriggerQuest2");
	TestTrue("Dependent quest should NOW be auto accepted", Progression->IsQuestAccepted("Q_SuccessDeps"));

	return true;
}

const FString FailureDependentQuestJson = R"RAWJSON([
	{
		"Identifier": "Q_FailureDeps",
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"FailureDepQuestTitle\", \"FailureDep Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"FailureDepQuestDesc\", \"The smallest possible quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": true,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": ["Q_TriggerQuest1", "Q_TriggerQuest2"],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"FailureDepObjTitle\", \"Small Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"FailureDepObjDesc\", \"This is super easy\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_FailureDep",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"FailureDepQuestTaskTitle\", \"Easiest possible thing\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
])RAWJSON";

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestFailureDependencies, "SUQSTest.QuestFailureDependencies",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestFailureDependencies::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
        	USuqsProgression::MakeQuestDataTableFromJSON(TriggerQuestsJson),
        	USuqsProgression::MakeQuestDataTableFromJSON(FailureDependentQuestJson)
        }
    );

	// Accept the 2 trigger quests
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_TriggerQuest1"));
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_TriggerQuest2"));

	TestFalse("Dependent quest must not be accepted yet", Progression->IsQuestAccepted("Q_FailureDeps"));
	// complete one trigger quest
	Progression->CompleteQuest("Q_TriggerQuest1");
	TestFalse("Dependent quest must not be accepted yet", Progression->IsQuestAccepted("Q_FailureDeps"));
	// fail the other one
	Progression->FailQuest("Q_TriggerQuest2");
	TestFalse("Dependent quest must not be accepted yet", Progression->IsQuestAccepted("Q_FailureDeps"));
	// now reset first
	Progression->ResetQuest("Q_TriggerQuest1");
	TestFalse("Dependent quest must not be accepted yet", Progression->IsQuestAccepted("Q_FailureDeps"));
	// Fail the second, should trigger now
	Progression->FailQuest("Q_TriggerQuest1");
	TestTrue("Dependent quest should NOW be auto accepted", Progression->IsQuestAccepted("Q_FailureDeps"));

	return true;
}

const FString MixedDependentQuestJson = R"RAWJSON([
	{
		"Identifier": "Q_MixedDeps",
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"MixedDepQuestTitle\", \"MixedDep Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"MixedDepQuestDesc\", \"The smallest possible quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": true,
		"PrerequisiteQuests": ["Q_TriggerQuest2"],
		"PrerequisiteQuestFailures": ["Q_TriggerQuest1"],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"MixedDepObjTitle\", \"Small Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"MixedDepObjDesc\", \"This is super easy\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_MixedDep",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"MixedDepQuestTaskTitle\", \"Easiest possible thing\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
])RAWJSON";

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestMixedDependencies, "SUQSTest.QuestMixedDependencies",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestMixedDependencies::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
		TArray<UDataTable*>{
			USuqsProgression::MakeQuestDataTableFromJSON(TriggerQuestsJson),
			USuqsProgression::MakeQuestDataTableFromJSON(MixedDependentQuestJson)
		}
	);

	// Accept the 2 trigger quests
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_TriggerQuest1"));
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_TriggerQuest2"));

	TestFalse("Dependent quest must not be accepted yet", Progression->IsQuestAccepted("Q_MixedDeps"));
	// complete one trigger quest
	Progression->CompleteQuest("Q_TriggerQuest2");
	TestFalse("Dependent quest must not be accepted yet", Progression->IsQuestAccepted("Q_MixedDeps"));
	// fail the other one, should trigger as we need mixed results
	Progression->FailQuest("Q_TriggerQuest1");
	TestTrue("Dependent quest should NOW be auto accepted", Progression->IsQuestAccepted("Q_MixedDeps"));

	return true;
}