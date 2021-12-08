#pragma once

// Yes "Name" and "Identifier" are duplicated in the JSON for quests
// That's because UE *requires* that you have a "Name" property, but then doesn't let you see what it is
// in the structure itself (only via the datatable). Barring copying it across to another structure at import,
// this means we need the Identifier separately but also a unique Name. It doesn't actually matter what the name is.

const FString SimpleMainQuestJson = R"RAWJSON([
	{
		"Identifier": "Q_Main1",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"TestQuests\", \"MainQuest1Title\", \"Main Quest 1\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"MainQuest1DescActive\", \"This is the main quest\")",
		"DescriptionWhenCompleted": "NSLOCTEXT(\"TestQuests\", \"MainQuest1DescComplete\", \"You did the main quest!\")",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"TestQuests\", \"FirstObjectiveTitle\", \"First objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"FirstObjectiveDesc\", \"This is the thing you do first\")",
				"DescriptionWhenCompleted": "NSLOCTEXT(\"TestQuests\", \"FirstObjectiveDescComplete\", \"You did the thing you do first!\")",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_ReachThePlace",
						"Title": "NSLOCTEXT(\"TestQuests\", \"ReachThePlace\", \"Reach The Place\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_DoTheThing",
						"Title": "NSLOCTEXT(\"TestQuests\", \"DoTheThing\", \"Do the thing\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_CollectDoobries",
						"Title": "NSLOCTEXT(\"TestQuests\", \"CollectSomeDoobries\", \"Collect Some Doobries\")",
						"bMandatory": false,
						"TargetNumber": 5,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O2",
				"Title": "NSLOCTEXT(\"TestQuests\", \"SecondObjectiveTitle\", \"Second objective: non-ordered tasks\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"SecondObjectiveDesc\", \"This is the second set of things\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": false,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_Something1",
						"Title": "NSLOCTEXT(\"TestQuests\", \"DoSomething\", \"Do Something\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_Something2",
						"Title": "NSLOCTEXT(\"TestQuests\", \"DoSomethingElse\", \"Do Something Else\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O3",
				"Title": "NSLOCTEXT(\"TestQuests\", \"LastObjectiveTitle\", \"Last Objective: A or B\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"LastObjectiveDesc\", \"Finally, you only have to do one of the tasks below\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": false,
				"NumberOfMandatoryTasksRequired": 1,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "TOptionA",
						"Title": "NSLOCTEXT(\"TestQuests\", \"DoThis\", \"Do this OR\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "TOptionB",
						"Title": "NSLOCTEXT(\"TestQuests\", \"DoThat\", \"Do That\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
	
	{
		"Identifier": "Q_Main2",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"TestQuests\", \"MainQuest2Title\", \"Main Quest 2\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"MainQuest2DescActive\", \"Second quest in the main quest\")",
		"DescriptionWhenCompleted": "NSLOCTEXT(\"TestQuests\", \"MainQuest2DescComplete\", \"You did the second part of the main quest!\")",
		"AutoAccept": true,
		"PrerequisiteQuests": ["Q_Main1"],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"TestQuests\", \"MainQ2FirstObjectiveTitle\", \"First objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"MainQ2FirstObjectiveDesc\", \"This is the thing you do first\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_DoTheThing",
						"Title": "NSLOCTEXT(\"TestQuests\", \"DoTheThing\", \"Do the thing\")",
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
		"Identifier": "Q_Side1",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"TestQuests\", \"SideQuest1Title\", \"Side Quest 1\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"SideQuest1DescActive\", \"This is one of those side quests you really play the game for\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"TestQuests\", \"MeetTheWizard\", \"Meet The Wizard\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"MeetTheWizardDesc\", \"So this is where you go meet the wizard\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_ReachWizardLand",
						"Title": "NSLOCTEXT(\"TestQuests\", \"ArriveAtWizardshire\", \"Arrive At Wizardshire\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_EnterTower",
						"Title": "NSLOCTEXT(\"TestQuests\", \"EnterTheTower\", \"Enter the tower\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_TalkToWizard",
						"Title": "NSLOCTEXT(\"TestQuests\", \"TalkToDerekTheWizard\", \"Talk to Derek the Wizard\")",
						"bMandatory": false,
						"TargetNumber": 5,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O2",
				"Title": "NSLOCTEXT(\"TestQuests\", \"KillDerek\", \"Kill Derek The Asshole Wizard\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"KillDerekDesc\", \"Turns out Derek is an asshole. Sort him out.\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_DropShield",
						"Title": "NSLOCTEXT(\"TestQuests\", \"DropDereksShield\", \"Make Derek Drop His Shields\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_MercDerek",
						"Title": "NSLOCTEXT(\"TestQuests\", \"MercDerek\", \"Merc Derek\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_InsultDerek",
						"Title": "NSLOCTEXT(\"TestQuests\", \"InsultDerek\", \"Call Derek's Lineage Into Question\")",
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
		"Identifier": "Q_Smol",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"TestQuests\", \"SmolQuestTitle\", \"Smol Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"SmolQuestDesc\", \"The smallest possible quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Title": "NSLOCTEXT(\"TestQuests\", \"SmolObjTitle\", \"Small Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"SmolObjDesc\", \"This is super easy\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_Smol",
						"Title": "NSLOCTEXT(\"TestQuests\", \"SmolQuestTaskTitle\", \"Easiest possible thing\")",
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
		"Identifier": "Q_Ordered",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"TestQuests\", \"OrderedQuestTitle\", \"Ordered Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"OrderedQuestDesc\", \"Ordered quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"TestQuests\", \"OrderedObjTitle\", \"Ordered Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"OrderedObjDesc\", \"These have to be done in order\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_1",
						"Title": "NSLOCTEXT(\"TestQuests\", \"OrderedQuestTask1Title\", \"Task 1\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_2",
						"Title": "NSLOCTEXT(\"TestQuests\", \"OrderedQuestTask2Title\", \"Task 2\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_3",
						"Title": "NSLOCTEXT(\"TestQuests\", \"OrderedQuestTask3Title\", \"Task 3\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O2",
				"Title": "NSLOCTEXT(\"TestQuests\", \"OrderedObjTitle\", \"Ordered Objective 2\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"OrderedObjDesc\", \"These have to be done in order\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_11",
						"Title": "NSLOCTEXT(\"TestQuests\", \"OrderedQuestTask1Title\", \"Task 1\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_12",
						"Title": "NSLOCTEXT(\"TestQuests\", \"OrderedQuestTask2Title\", \"Task 2\")",
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
		"Identifier": "Q_Unordered",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"TestQuests\", \"UnorderedQuestTitle\", \"Unordered Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"UnorderedQuestDesc\", \"Unordered quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Title": "NSLOCTEXT(\"TestQuests\", \"UnorderedObjTitle\", \"Unordered Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"UnorderedObjDesc\", \"These can be done in any order, but still all of them\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": false,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_1",
						"Title": "NSLOCTEXT(\"TestQuests\", \"UnorderedQuestTask1Title\", \"Task 1\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_2",
						"Title": "NSLOCTEXT(\"TestQuests\", \"UnorderedQuestTask2Title\", \"Task 2\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_3",
						"Title": "NSLOCTEXT(\"TestQuests\", \"UnorderedQuestTask3Title\", \"Task 3\")",
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
		"Identifier": "Q_Any2Of",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"TestQuests\", \"AnyOfQuestTitle\", \"AnyOf Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"AnyOfQuestDesc\", \"AnyOf quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Title": "NSLOCTEXT(\"TestQuests\", \"Any2OfObjTitle\", \"Any2Of Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"Any2OfObjDesc\", \"2 tasks need to be done\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": false,
				"NumberOfMandatoryTasksRequired": 2,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_1",
						"Title": "NSLOCTEXT(\"TestQuests\", \"AnyOfQuestTask1Title\", \"Task 1\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_2",
						"Title": "NSLOCTEXT(\"TestQuests\", \"AnyOfQuestTask2Title\", \"Task 2\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_3",
						"Title": "NSLOCTEXT(\"TestQuests\", \"AnyOfQuestTask3Title\", \"Task 3\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_Optional",
						"Title": "NSLOCTEXT(\"TestQuests\", \"AnyOfQuestTask4Title\", \"Optional Task\")",
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
		"Identifier": "Q_TargetNumbers",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"TestQuests\", \"TargetNumberQuestTitle\", \"TargetNumber Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"TargetNumberQuestDesc\", \"TargetNumber quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Title": "NSLOCTEXT(\"TestQuests\", \"TargetNumberObjTitle\", \"TargetNumber Objective\")",
				"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"TargetNumberObjDesc\", \"These have to be done in order\")",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_TargetOf1",
						"Title": "NSLOCTEXT(\"TestQuests\", \"TargetNumberQuestTask1Title\", \"Task 1\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_TargetOf3",
						"Title": "NSLOCTEXT(\"TestQuests\", \"TargetNumberQuestTask2Title\", \"Task 2\")",
						"bMandatory": true,
						"TargetNumber": 3,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_TargetOf6",
						"Title": "NSLOCTEXT(\"TestQuests\", \"TargetNumberQuestTask3Title\", \"Task 3\")",
						"bMandatory": true,
						"TargetNumber": 6,
						"TimeLimit": 0
					}
				]
			}
		]
	},
])RAWJSON";

const FString BranchingQuestJson = R"RAWJSON([
	{
		"Identifier": "Q_Branching",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"TestQuests\", \"BranchineQuestTitle\", \"Branching Quest\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"BranchingQuestDesc\", \"A branching quest\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O1",
				"Title": "NSLOCTEXT(\"TestQuests\", \"BObj1Title\", \"A common objective (all branches)\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_1",
						"Title": "NSLOCTEXT(\"TestQuests\", \"BQT1Desc\", \"It's a common task\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O_BranchA_1",
				"Title": "NSLOCTEXT(\"TestQuests\", \"BObjBA1Title\", \"First objective on branch A\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "BranchA",
				"Tasks": [
					{
						"Identifier": "T_BA_1",
						"Title": "NSLOCTEXT(\"TestQuests\", \"BQTBA1Desc\", \"This is the only task on BranchA\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O_BranchB_1",
				"Title": "NSLOCTEXT(\"TestQuests\", \"BObjBB1Title\", \"First objective on branch B\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "BranchB",
				"Tasks": [
					{
						"Identifier": "T_BB_1",
						"Title": "NSLOCTEXT(\"TestQuests\", \"BQTBB1Desc\", \"This is task 1 on BranchB\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O_BranchB_2",
				"Title": "NSLOCTEXT(\"TestQuests\", \"BObjBB2Title\", \"Second objective on branch B\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "BranchB",
				"Tasks": [
					{
						"Identifier": "T_BB_2",
						"Title": "NSLOCTEXT(\"TestQuests\", \"BQTBB2Desc\", \"This is task 1 on BranchB\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},
			{
				"Identifier": "O3",
				"Title": "NSLOCTEXT(\"TestQuests\", \"BObj3Title\", \"Finishing objective (common to all branches)\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_3",
						"Title": "NSLOCTEXT(\"TestQuests\", \"BQT3Desc\", \"Common task to finish\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			},

		]
	},
])RAWJSON";

const FString TimeLimitQuestJson = R"RAWJSON([
	{
		"Identifier": "Q_TimeLimits",
		"bPlayerVisible": true,
		"Title": "NSLOCTEXT(\"TestQuests\", \"TimeLimitQuestTitle\", \"Quest With Time Limit\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"TimeLimitQuestDesc\", \"A quest with a time limit\")",
		"DescriptionWhenCompleted": "",
		"AutoAccept": false,
		"PrerequisiteQuests": [],
		"PrerequisiteQuestFailures": [],
		"Objectives": [
			{
				"Identifier": "O_Single",
				"Title": "NSLOCTEXT(\"TestQuests\", \"TimeLimitO1\", \"First objective, single task\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_Single",
						"Title": "NSLOCTEXT(\"TestQuests\", \"TSingleDesc\", \"Single time limit task\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 100
					}
				]
			},
			{
				"Identifier": "O_SingleButNotFirst",
				"Title": "NSLOCTEXT(\"TestQuests\", \"TimeLimitO2\", \"Objective with 2 sequential tasks, later one is timed\")",
				"DescriptionWhenActive": "",
				"DescriptionWhenCompleted": "",
				"bSequentialTasks": true,
				"Branch": "None",
				"Tasks": [
					{
						"Identifier": "T_NonTimeLimited",
						"Title": "NSLOCTEXT(\"TestQuests\", \"TNotTimeLimited\", \"This is not time limited\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					},
					{
						"Identifier": "T_SecondTimeLimited",
						"Title": "NSLOCTEXT(\"TestQuests\", \"TTimeLimited\", \"This is time limited\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 50
					},
					{
						"Identifier": "T_NonTimeLimited2",
						"Title": "NSLOCTEXT(\"TestQuests\", \"TNotTimeLimited\", \"This is not time limited\")",
						"bMandatory": true,
						"TargetNumber": 1,
						"TimeLimit": 0
					}
				]
			}
		]
	},
])RAWJSON";

const FString NonAutoResolveQuestsJson = R"RAWJSON([
{
    "Identifier": "Q_TimedResolve",
    "bPlayerVisible": true,
    "Title": "NSLOCTEXT(\"TestQuests\", \"TimedResolveTitle\", \"Quest With Resolve Time\")",
    "DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"TimedResolveDesc\", \"A quest with a resolve time\")",
    "DescriptionWhenCompleted": "",
    "AutoAccept": false,
    "PrerequisiteQuests": [],
    "PrerequisiteQuestFailures": [],
    "ResolveDelay": 1,
    "Objectives": [
        {
            "Identifier": "O_Single",
            "Title": "NSLOCTEXT(\"TestQuests\", \"TimedResolveO1\", \"First objective, single task\")",
            "Tasks": [
                {
                    "Identifier": "T_Single",
                    "Title": "NSLOCTEXT(\"TestQuests\", \"TSingleDesc\", \"Timed resolve task\")",
                    "bMandatory": true,
                    "ResolveDelay": 3
                }
            ]
        }
    ]
},
{
    "Identifier": "Q_GatedResolve",
    "bPlayerVisible": true,
    "Title": "NSLOCTEXT(\"TestQuests\", \"GatedResolveTitle\", \"Quest With Resolve Gate\")",
    "DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"GatedResolveDesc\", \"A quest with a resolve gate\")",
    "DescriptionWhenCompleted": "",
    "AutoAccept": false,
    "PrerequisiteQuests": [],
    "PrerequisiteQuestFailures": [],
    "ResolveGate": "TestGate",
    "Objectives": [
        {
            "Identifier": "O_Single",
            "Title": "NSLOCTEXT(\"TestQuests\", \"GatedResolveO1\", \"First objective, single task\")",
            "Tasks": [
                {
                    "Identifier": "T_Single",
                    "Title": "NSLOCTEXT(\"TestQuests\", \"TSingleDesc\", \"Gated resolve task\")",
                    "bMandatory": true,
                    "ResolveGate": "TestGate"
                }
            ]
        }
    ]
},
{
    "Identifier": "Q_ManualResolve",
    "bPlayerVisible": true,
    "Title": "NSLOCTEXT(\"TestQuests\", \"ManualResolveTitle\", \"Quest With Resolve Time\")",
    "DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"ManualResolveDesc\", \"A quest with manual resolve\")",
    "DescriptionWhenCompleted": "",
    "AutoAccept": false,
    "PrerequisiteQuests": [],
    "PrerequisiteQuestFailures": [],
    "bResolveAutomatically": false,
    "Objectives": [
        {
            "Identifier": "O_Single",
            "Title": "NSLOCTEXT(\"TestQuests\", \"ManualResolveO1\", \"First objective, single task\")",
            "Tasks": [
                {
                    "Identifier": "T_Single",
                    "Title": "NSLOCTEXT(\"TestQuests\", \"TSingleDesc\", \"Manual resolve task\")",
                    "bMandatory": true,
                    "bResolveAutomatically": false
                }
            ]
        }
    ]
}
])RAWJSON";
