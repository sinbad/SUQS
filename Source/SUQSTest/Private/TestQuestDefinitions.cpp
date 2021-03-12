#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine.h"
#include "SuqsPlayState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestCreateQuestDefinitions, "SUQSTest.CreateQuestDefinitions",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestCreateQuestDefinitions::RunTest(const FString& Parameters)
{
	USuqsPlayState* PlayState = NewObject<USuqsPlayState>();
	UDataTable* QuestTable = NewObject<UDataTable>();

	QuestTable->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable->CreateTableFromJSONString(R"RAWJSON([
	{
		"Name": "MainQuest1Row",
		"Identifier": "Q_Main1",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1Title\", \"Main Quest 1\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1DescActive\", \"This is the main quest\")",
		"DescriptionWhenCompleted": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1DescComplete\", \"You did the main quest!\")",
		"AutoAccept": false,
		"PrerequiteQuests": [],
		"PrerequiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"FirstObjectiveTitle\", \"First objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"FirstObjectiveDesc\", \"This is the thing you do first\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_ReachThePlace",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"ReachThePlace\", \"Reach The Place\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_DoTheThing",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"DoTheThing\", \"Do the thing\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_CollectDoobries",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"CollectSomeDoobries\", \"Collect Some Doobries\")",
						"bMandatory": false,
						"TargetNumber": 5,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O2",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"SecondObjectiveTitle\", \"Second objective: non-ordered tasks\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"SecondObjectiveDesc\", \"This is the second set of things\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": false,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_Something1",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"DoSomething\", \"Do Something\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_Something2",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"DoSomethingElse\", \"Do Something Else\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O3",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"LastObjectiveTitle\", \"Last Objective: A or B\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"LastObjectiveDesc\", \"Finally, you only have to do one of the tasks below\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": false,
				"bAllMandatoryTasksRequired": false,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "TOptional1",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"DoThis\", \"Do this OR\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "TOptional2",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"DoThat\", \"Do That\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
	
	{
		"Name": "MainQuest2Row",
		"Identifier": "Q_Main2",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest2Title\", \"Main Quest 2\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1DescActive\", \"Second quest in the main quest\")",
		"DescriptionWhenCompleted": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1DescComplete\", \"You did the secon part of the main quest!\")",
		"AutoAccept": false,
		"PrerequiteQuests": [],
		"PrerequiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"FirstObjectiveTitle\", \"First objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"FirstObjectiveDesc\", \"This is the thing you do first\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_DoTheThing",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"DoTheThing\", \"Do the thing\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
	])RAWJSON");

	if (!TestEqual("Should have 2 quests in table", QuestTable->GetRowMap().Num(), 2))
		return false;

	return true;
}
