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
		"Name": "Q_Main2",
		"Identifier": "Q_Main2",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest2Title\", \"Main Quest 2\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1DescActive\", \"Second quest in the main quest\")",
		"DescriptionWhenCompleted": "NSLOCTEXT(\"[TestQuests]\", \"MainQuest1DescComplete\", \"You did the secon part of the main quest!\")",
		"AutoAccept": true,
		"PrerequiteQuests": ["Q_Main1"],
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
		"PrerequiteQuests": [],
		"PrerequiteQuestFailures": [],
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
					}
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
