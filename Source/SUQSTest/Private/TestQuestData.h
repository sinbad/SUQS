#pragma once

// Yes "Name" and "Identifier" are duplicated in the JSON for quests
// That's because UE *requires* that you have a "Name" property, but then doesn't let you see what it is
// in the structure itself (only via the datatable). Barring copying it across to another structure at import,
// this means we need the Identifier separately but also a unique Name. It doesn't actually matter what the name is.

const FString SimpleMainQuestJson = R"RAWJSON([
	{
		"Name": "Q_Main1",
		"Identifier": "Q_Main1",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1Title\", \"Main Quest 1\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1DescActive\", \"This is the main quest\")",
		"DescriptionWhenCompleted": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1DescComplete\", \"You did the main quest!\")",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
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
						"Identifier": "TOptionA",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"DoThis\", \"Do this OR\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "TOptionB",
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
		"Name": "Q_Main2",
		"Identifier": "Q_Main2",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest2Title\", \"Main Quest 2\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1DescActive\", \"Second quest in the main quest\")",
		"DescriptionWhenCompleted": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1DescComplete\", \"You did the secon part of the main quest!\")",
		"AutoAccept": true,
		"PrerequisiteQuests": ["Q_Main1"],
		"PrerequisiteQuestFailures": [],
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
	}
])RAWJSON";


const FString SimpleSideQuestJson = R"RAWJSON([
	{
		"Name": "Q_Side1",
		"Identifier": "Q_Side1",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"SideQuest1Title\", \"Side Quest 1\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"SideQuest1DescActive\", \"This is one of those side quests you really play the game for\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"MeetTheWizard\", \"Meet The Wizard\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"MeetTheWizardDesc\", \"So this is where you go meet the wizard\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_ReachWizardLand",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"ArriveAtWizardshire\", \"Arrive At Wizardshire\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_EnterTower",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"EnterTheTower\", \"Enter the tower\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_TalkToWizard",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"TalkToDerekTheWizard\", \"Talk to Derek the Wizard\")",
						"bMandatory": false,
						"TargetNumber": 5,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O2",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"KillDerek\", \"Kill Derek The Asshole Wizard\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"KillDerekDesc\", \"Turns out Derek is an asshole. Sort him out.\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_DropShield",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"DropDereksShield\", \"Make Derek Drop His Shields\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_MercDerek",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"MercDerek\", \"Merc Derek\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_InsultDerek",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"InsultDerek\", \"Call Derek's Lineage Into Question\")",
						"bMandatory": false,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
])RAWJSON";

const FString SmallestPossibleQuestJson = R"RAWJSON([
	{
		"Name": "Q_Smol",
		"Identifier": "Q_Smol",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"SmolQuestTitle\", \"Smol Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"SmolQuestDesc\", \"The smallest possible quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"SmolObjTitle\", \"Small Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"SmolObjDesc\", \"This is super easy\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_Smol",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"SmolQuestTaskTitle\", \"Easiest possible thing\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
])RAWJSON";

const FString OrderedTasksQuestJson = R"RAWJSON([
	{
		"Name": "Q_Ordered",
		"Identifier": "Q_Ordered",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"OrderedQuestTitle\", \"Ordered Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"OrderedQuestDesc\", \"Ordered quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"OrderedObjTitle\", \"Ordered Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"OrderedObjDesc\", \"These have to be done in order\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_1",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"OrderedQuestTask1Title\", \"Task 1\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_2",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"OrderedQuestTask2Title\", \"Task 2\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_3",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"OrderedQuestTask3Title\", \"Task 3\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O2",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"OrderedObjTitle\", \"Ordered Objective 2\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"OrderedObjDesc\", \"These have to be done in order\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_11",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"OrderedQuestTask1Title\", \"Task 1\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_12",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"OrderedQuestTask2Title\", \"Task 2\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
])RAWJSON";

const FString UnorderedTasksQuestJson = R"RAWJSON([
	{
		"Name": "Q_Unordered",
		"Identifier": "Q_Unordered",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"UnorderedQuestTitle\", \"Unordered Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"UnorderedQuestDesc\", \"Unordered quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"UnorderedObjTitle\", \"Unordered Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"UnorderedObjDesc\", \"These can be done in any order, but still all of them\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": false,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_1",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"UnorderedQuestTask1Title\", \"Task 1\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_2",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"UnorderedQuestTask2Title\", \"Task 2\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_3",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"UnorderedQuestTask3Title\", \"Task 3\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
])RAWJSON";


const FString AnyOfTasksQuestJson = R"RAWJSON([
	{
		"Name": "Q_AnyOf",
		"Identifier": "Q_AnyOf",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"AnyOfQuestTitle\", \"AnyOf Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"AnyOfQuestDesc\", \"AnyOf quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"AnyOfObjTitle\", \"AnyOf Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"AnyOfObjDesc\", \"Only one of these tasks needs to be done\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": false,
				"bAllMandatoryTasksRequired": false,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_1",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"AnyOfQuestTask1Title\", \"Task 1\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_2",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"AnyOfQuestTask2Title\", \"Task 2\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_3",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"AnyOfQuestTask3Title\", \"Task 3\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_Optional",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"AnyOfQuestTask4Title\", \"Optional Task\")",
						"bMandatory": false,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
])RAWJSON";

const FString TargetNumberQuestJson = R"RAWJSON([
	{
		"Name": "Q_TargetNumbers",
		"Identifier": "Q_TargetNumbers",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"TargetNumberQuestTitle\", \"TargetNumber Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"TargetNumberQuestDesc\", \"TargetNumber quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"[TestQuests]\", \"TargetNumberObjTitle\", \"TargetNumber Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"TargetNumberObjDesc\", \"These have to be done in order\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"bAllMandatoryTasksRequired": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_TargetOf1",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"TargetNumberQuestTask1Title\", \"Task 1\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_TargetOf3",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"TargetNumberQuestTask2Title\", \"Task 2\")",
						"bMandatory": true,
						"TargetNumber": 3,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_TargetOf6",
						"Title": "NSLOCTEXT(\"[TestQuests]\", \"TargetNumberQuestTask3Title\", \"Task 3\")",
						"bMandatory": true,
						"TargetNumber": 6,
						"TimeLimit": 0
					}
				]
			}
		]
	},
])RAWJSON";
