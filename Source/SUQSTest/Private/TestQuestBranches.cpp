#include "Misc/AutomationTest.h"
#include "Engine.h"
#include "SuqsObjectiveState.h"
#include "SuqsProgression.h"
#include "SuqsTaskState.h"

const FString BranchingQuestJson = R"RAWJSON([
	{
		"Name": "Q_Branching",
		"Identifier": "Q_Branching",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"BranchineQuestTitle\", \"Branching Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"BranchingQuestDesc\", \"A branching quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"BObj1Title\", \"A common objective (all branches)\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_1",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"BQT1Desc\", \"It's a common task\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O_BranchA_1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"BObjBA1Title\", \"First objective on branch A\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "BranchA",
				"Tasks": [
					{
						"Identifier": "T_BA_1",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"BQTBA1Desc\", \"This is the only task on BranchA\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O_BranchB_1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"BObjBB1Title\", \"First objective on branch B\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "BranchB",
				"Tasks": [
					{
						"Identifier": "T_BB_1",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"BQTBB1Desc\", \"This is task 1 on BranchB\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O_BranchB_2",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"BObjBB2Title\", \"Second objective on branch B\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "BranchB",
				"Tasks": [
					{
						"Identifier": "T_BB_2",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"BQTBB2Desc\", \"This is task 1 on BranchB\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O3",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"BObj3Title\", \"Finishing objective (common to all branches)\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_3",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"BQT3Desc\", \"Common task to finish\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},

		]
	},
])RAWJSON";

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestBranchNoBranches, "SUQSTest.QuestBranchNoBranches",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestBranchNoBranches::RunTest(const FString& Parameters)
{
	// This tests what happens with no branches enabled
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	UDataTable* QuestTable = NewObject<UDataTable>();
	QuestTable->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable->CreateTableFromJSONString(BranchingQuestJson);

	Progression->QuestDataTables.Add(QuestTable);

	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_Branching"));
	auto Q = Progression->GetQuest("Q_Branching");
	TestEqual("Next objective should be O1", Q->GetCurrentObjective()->GetIdentifier(), FName("O1"));
	TestTrue("Task completion should be OK", Q->CompleteTask("T_1"));
	TestEqual("With no branches, next objective should be O3", Q->GetCurrentObjective()->GetIdentifier(), FName("O3"));
	TestTrue("Quest should be incomplete", Q->IsIncomplete());
	TestTrue("Task completion should be OK", Q->CompleteTask("T_3"));
	TestTrue("Quest should be complete", Q->IsCompleted());
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestBranchA, "SUQSTest.QuestBranchA",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestBranchA::RunTest(const FString& Parameters)
{
	// This tests what happens with no branches enabled
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	UDataTable* QuestTable = NewObject<UDataTable>();
	QuestTable->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable->CreateTableFromJSONString(BranchingQuestJson);

	Progression->QuestDataTables.Add(QuestTable);

	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_Branching"));
	auto Q = Progression->GetQuest("Q_Branching");

	Q->SetBranchActive("BranchA", true);
	
	TestEqual("Next objective should be O1", Q->GetCurrentObjective()->GetIdentifier(), FName("O1"));
	TestTrue("Task completion should be OK", Q->CompleteTask("T_1"));
	TestEqual("On BranchA, next objective should be O_BranchA_1", Q->GetCurrentObjective()->GetIdentifier(), FName("O_BranchA_1"));
	TestTrue("Task completion should be OK", Q->CompleteTask("T_BA_1"));
	TestEqual("Next objective should be O3", Q->GetCurrentObjective()->GetIdentifier(), FName("O3"));
	TestTrue("Task completion should be OK", Q->CompleteTask("T_3"));
	TestTrue("Quest should be complete", Q->IsCompleted());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestBranchB, "SUQSTest.QuestBranchB",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestBranchB::RunTest(const FString& Parameters)
{
	// This tests what happens with no branches enabled
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	UDataTable* QuestTable = NewObject<UDataTable>();
	QuestTable->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable->CreateTableFromJSONString(BranchingQuestJson);

	Progression->QuestDataTables.Add(QuestTable);

	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_Branching"));
	auto Q = Progression->GetQuest("Q_Branching");

	Q->SetBranchActive("BranchB", true);
	
	TestEqual("Next objective should be O1", Q->GetCurrentObjective()->GetIdentifier(), FName("O1"));
	TestTrue("Task completion should be OK", Q->CompleteTask("T_1"));
	TestEqual("On BranchB, next objective should be O_BranchB_1", Q->GetCurrentObjective()->GetIdentifier(), FName("O_BranchB_1"));
	TestTrue("Task completion should be OK", Q->CompleteTask("T_BB_1"));
	TestEqual("Next task should be second on BranchB", Q->GetCurrentObjective()->GetNextMandatoryTask()->GetIdentifier(), FName("T_BB_2"));
	TestTrue("Task completion should be OK", Q->CompleteTask("T_BB_2"));
	TestEqual("Next objective should be O3", Q->GetCurrentObjective()->GetIdentifier(), FName("O3"));
	TestTrue("Task completion should be OK", Q->CompleteTask("T_3"));
	TestTrue("Quest should be complete", Q->IsCompleted());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestDynamicBranchChange, "SUQSTest.QuestDynamicBranchChange",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestDynamicBranchChange::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	UDataTable* QuestTable = NewObject<UDataTable>();
	QuestTable->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable->CreateTableFromJSONString(BranchingQuestJson);

	Progression->QuestDataTables.Add(QuestTable);

	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_Branching"));
	auto Q = Progression->GetQuest("Q_Branching");

	// Start on branch B
	Q->SetBranchActive("BranchB", true);
	TestEqual("Next objective should be O1", Q->GetCurrentObjective()->GetIdentifier(), FName("O1"));
	TestTrue("Task completion should be OK", Q->CompleteTask("T_1"));
	TestEqual("On BranchB, next objective should be O_BranchB_1", Q->GetCurrentObjective()->GetIdentifier(), FName("O_BranchB_1"));
	// partially complete branch B
	TestTrue("Task completion should be OK", Q->CompleteTask("T_BB_1"));

	// now, change to branch A
	// first disable branch B
	Q->SetBranchActive("BranchB", false);
	TestEqual("After disabling BranchB, next objective should be O3", Q->GetCurrentObjective()->GetIdentifier(), FName("O3"));
	// now enable branch A
	Q->SetBranchActive("BranchA", true);
	TestEqual("After enabling BranchA, next objective should be O_BranchA_1", Q->GetCurrentObjective()->GetIdentifier(), FName("O_BranchA_1"));
	// now let's go back, and test that we remembered that we completed the first BranchB objective
	Q->SetBranchActive("BranchA", false);
	Q->SetBranchActive("BranchB", true);
	TestEqual("Back on BranchB, next objective should be O_BranchB_2", Q->GetCurrentObjective()->GetIdentifier(), FName("O_BranchB_2"));

	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestGlobalBranch, "SUQSTest.QuestGlobalBranch",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestGlobalBranch::RunTest(const FString& Parameters)
{
	// TODO
	return false;
}
