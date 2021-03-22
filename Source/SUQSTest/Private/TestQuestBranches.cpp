#include "Misc/AutomationTest.h"
#include "Engine.h"
#include "SuqsObjectiveState.h"
#include "SuqsProgression.h"
#include "SuqsTaskState.h"
#include "TestQuestData.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestBranchNoBranches, "SUQSTest.QuestBranchNoBranches",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestBranchNoBranches::RunTest(const FString& Parameters)
{
	// This tests what happens with no branches enabled
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(BranchingQuestJson)
        }
    );


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
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(BranchingQuestJson)
        }
    );

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
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(BranchingQuestJson)
        }
    );

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
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(BranchingQuestJson)
        }
    );


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


// Create 2 more quests - they're basically the same but that doesn't matter, we're just testing global branch changes
const FString BranchingQuest2Json = R"RAWJSON([
    {
        "Identifier": "Q_Branching2",
        "bPlayerVisible": true,
        "Title": "NSLOCTEXT(\"[TestQuests]\", \"Branching2QuestTitle\", \"Branching Quest 2\")",
        "DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"BranchingQuestDesc\", \"Another branching quest\")",
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
                "Identifier": "O_BranchA_Q2",
                "Title": "NSLOCTEXT(\"[TestQuests]\", \"BObjBA1Title\", \"First objective on branch A\")",
                "DescriptionWhenActive": "",
                "DescriptionWhenCompleted": "",
                "bSequentialTasks": true,
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
				"Identifier": "O_BranchB_Q2",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"BObjBB1Title\", \"First objective on branch B\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
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
			}
        ]
    },
    {
        "Identifier": "Q_Branching3",
        "bPlayerVisible": true,
        "Title": "NSLOCTEXT(\"[TestQuests]\", \"Branching2QuestTitle\", \"Branching Quest 2\")",
        "DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"BranchingQuestDesc\", \"Another branching quest\")",
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
                "Identifier": "O_BranchA_Q3",
                "Title": "NSLOCTEXT(\"[TestQuests]\", \"BObjBA1Title\", \"First objective on branch A\")",
                "DescriptionWhenActive": "",
                "DescriptionWhenCompleted": "",
                "bSequentialTasks": true,
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
				"Identifier": "O_BranchB_Q3",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"BObjBB1Title\", \"First objective on branch B\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
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
			}
        ]
    }
])RAWJSON";
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestGlobalBranch, "SUQSTest.QuestGlobalBranch",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestGlobalBranch::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
        TArray<UDataTable*> {
            USuqsProgression::MakeQuestDataTableFromJSON(BranchingQuestJson),
        	USuqsProgression::MakeQuestDataTableFromJSON(BranchingQuest2Json)
        }
    );

	TArray<USuqsObjectiveState*> ActiveObjectives;


	// Initially accept 2 quests
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_Branching"));
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_Branching2"));

	auto Q1 = Progression->GetQuest("Q_Branching");
	auto Q2 = Progression->GetQuest("Q_Branching2");

	Q1->GetActiveObjectives(ActiveObjectives);
	TestEqual("Should only be main branch active objectives", ActiveObjectives.Num(), 2);
	TestEqual("O3 should be second objective", ActiveObjectives[1]->GetIdentifier(), FName("O3"));
	// Q2 only has starter common objective
	Q2->GetActiveObjectives(ActiveObjectives);
	TestEqual("Should only be main branch active objectives", ActiveObjectives.Num(), 1);
	
	// Set BranchA enabled over all quests
	Progression->SetGlobalQuestBranchActive("BranchA", true);
	// Check that the second objective has now changed
	Q1->GetActiveObjectives(ActiveObjectives);
	TestEqual("Should now have BranchA objectives", ActiveObjectives.Num(), 3);
	TestEqual("BranchA should be second objective", ActiveObjectives[1]->GetIdentifier(), FName("O_BranchA_1"));
	// Q2 only has starter common objective
	Q2->GetActiveObjectives(ActiveObjectives);
	TestEqual("Should now have BranchA objectives", ActiveObjectives.Num(), 2);
	TestEqual("BranchA should be second objective", ActiveObjectives[1]->GetIdentifier(), FName("O_BranchA_Q2"));

	// Test that on accepting a new quest, it gets the current global branch settings
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_Branching3"));	
	auto Q3 = Progression->GetQuest("Q_Branching3");
	Q3->GetActiveObjectives(ActiveObjectives);
	TestEqual("Q3 should immediately have BranchA objectives", ActiveObjectives.Num(), 2);
	TestEqual("BranchA should be second objective", ActiveObjectives[1]->GetIdentifier(), FName("O_BranchA_Q3"));

	// Now let's switch
	Progression->SetGlobalQuestBranchActive("BranchB", true);
	Progression->SetGlobalQuestBranchActive("BranchA", false);

	// Check that the second objective has flipped to BranchB for all
	Q1->GetActiveObjectives(ActiveObjectives);
	// there are 2 objectives in BranchB compared to 1 in BranchA
	TestEqual("Should now have BranchB objectives", ActiveObjectives.Num(), 4);
	TestEqual("BranchB should be second objective on Q1", ActiveObjectives[1]->GetIdentifier(), FName("O_BranchB_1"));
	Q2->GetActiveObjectives(ActiveObjectives);
	TestEqual("Should now have BranchB objectives", ActiveObjectives.Num(), 2);
	TestEqual("BranchB should be second objective on Q2", ActiveObjectives[1]->GetIdentifier(), FName("O_BranchB_Q2"));
	Q3->GetActiveObjectives(ActiveObjectives);
	TestEqual("Should now have BranchB objectives", ActiveObjectives.Num(), 2);
	TestEqual("BranchB should be second objective on Q3", ActiveObjectives[1]->GetIdentifier(), FName("O_BranchB_Q3"));

	// Back to main branch
	Progression->ResetGlobalQuestBranches();

	Q1->GetActiveObjectives(ActiveObjectives);
	TestEqual("Should only be main branch active objectives", ActiveObjectives.Num(), 2);
	TestEqual("O3 should be second objective", ActiveObjectives[1]->GetIdentifier(), FName("O3"));
	// Q2/3 only has starter common objective
	Q2->GetActiveObjectives(ActiveObjectives);
	TestEqual("Should only be main branch active objectives", ActiveObjectives.Num(), 1);
	Q3->GetActiveObjectives(ActiveObjectives);
	TestEqual("Should only be main branch active objectives", ActiveObjectives.Num(), 1);
	
	
	return true;
}
