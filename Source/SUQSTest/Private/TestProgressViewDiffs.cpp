#include "SuqsProgressView.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestProgressViewDiffs,
                                 "SUQSTest.ProgressViewDiffs",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestProgressViewDiffs::RunTest(const FString& Parameters)
{
	FSuqsProgressView Before;
	FSuqsProgressView After;

	{
		auto& Q = After.ActiveQuests.AddDefaulted_GetRef();
		Q.Identifier = "Q_Test1";
		Q.Status = ESuqsQuestStatus::Incomplete;
		Q.CurrentObjectiveIdentifier = "O_1";
		Q.Description = INVTEXT("Some quest description");
		auto& T1 = Q.CurrentTasks.AddDefaulted_GetRef();
		T1.Identifier = "T_1";
		T1.Status = ESuqsTaskStatus::NotStarted;
		T1.bHidden = false;
		T1.Title = INVTEXT("Collect 3 things");
		T1.TargetNumber = 3;

		auto& T2 = Q.CurrentTasks.AddDefaulted_GetRef();
		T2.Identifier = "T_2";
		T2.Status = ESuqsTaskStatus::NotStarted;
		T2.bHidden = false;
		T2.Title = INVTEXT("Do something");
		T2.bMandatory = false;
	}

	FSuqsProgressViewDiff Diff;
	TestTrue("Should generate changes from new", USuqsProgressViewHelpers::GetProgressViewDifferences(Before, After, Diff));
	if (TestEqual("Should be 3 changes", Diff.Entries.Num(), 3))
	{
		TestEqual("First change should be new quest: category", Diff.Entries[0].Category, ESuqsProgressViewDiffCategory::Quest);
		TestEqual("First change should be new quest: change", Diff.Entries[0].ChangeType, ESuqsProgressViewDiffChangeType::Added);
		TestEqual("First change should be new quest: questid", Diff.Entries[0].QuestID, FName("Q_Test1"));
	
		TestEqual("Second change should be new Task: category", Diff.Entries[1].Category, ESuqsProgressViewDiffCategory::Task);
		TestEqual("Second change should be new Task: change", Diff.Entries[1].ChangeType, ESuqsProgressViewDiffChangeType::Added);
		TestEqual("Second change should be new Task: questid", Diff.Entries[1].QuestID, FName("Q_Test1"));
		TestEqual("Second change should be new Task: taskid", Diff.Entries[1].TaskID, FName("T_1"));
	
		TestEqual("Third change should be new Task: category", Diff.Entries[2].Category, ESuqsProgressViewDiffCategory::Task);
		TestEqual("Third change should be new Task: change", Diff.Entries[2].ChangeType, ESuqsProgressViewDiffChangeType::Added);
		TestEqual("Third change should be new Task: questid", Diff.Entries[2].QuestID, FName("Q_Test1"));
		TestEqual("Third change should be new Task: taskid", Diff.Entries[2].TaskID, FName("T_2"));
	}
	Before = After;

	// Add a new quest
	{
		auto& Q = After.ActiveQuests.AddDefaulted_GetRef();
		Q.Identifier = "Q_Test2";
		Q.Status = ESuqsQuestStatus::Incomplete;
		Q.CurrentObjectiveIdentifier = "O_1";
		Q.Description = INVTEXT("A second quest");
		auto& T1 = Q.CurrentTasks.AddDefaulted_GetRef();
		T1.Identifier = "T_1";
		T1.Status = ESuqsTaskStatus::NotStarted;
		T1.bHidden = false;
		T1.Title = INVTEXT("Do something");
	}
	
	TestTrue("Should generate changes from added quest", USuqsProgressViewHelpers::GetProgressViewDifferences(Before, After, Diff));
	if (TestEqual("Should be 2 changes", Diff.Entries.Num(), 2))
	{
		TestEqual("First change should be new quest: category", Diff.Entries[0].Category, ESuqsProgressViewDiffCategory::Quest);
		TestEqual("First change should be new quest: change", Diff.Entries[0].ChangeType, ESuqsProgressViewDiffChangeType::Added);
		TestEqual("First change should be new quest: questid", Diff.Entries[0].QuestID, FName("Q_Test2"));
	
		TestEqual("Second change should be new Task: category", Diff.Entries[1].Category, ESuqsProgressViewDiffCategory::Task);
		TestEqual("Second change should be new Task: change", Diff.Entries[1].ChangeType, ESuqsProgressViewDiffChangeType::Added);
		TestEqual("Second change should be new Task: questid", Diff.Entries[1].QuestID, FName("Q_Test2"));
		TestEqual("Second change should be new Task: taskid", Diff.Entries[1].TaskID, FName("T_1"));
	
	}
	Before = After;
	
	TestFalse("Should generate NO changes from same", USuqsProgressViewHelpers::GetProgressViewDifferences(Before, After, Diff));
	TestEqual("Should be NO changes", Diff.Entries.Num(), 0);

	// Modify a task
	After.ActiveQuests[0].CurrentTasks[0].CompletedNumber = 2;
	TestTrue("Should generate changes from modified task", USuqsProgressViewHelpers::GetProgressViewDifferences(Before, After, Diff));
	if (TestEqual("Should be 1 change", Diff.Entries.Num(), 1))
	{
		TestEqual("First change should be modified task: category", Diff.Entries[0].Category, ESuqsProgressViewDiffCategory::Task);
		TestEqual("First change should be modified task: change", Diff.Entries[0].ChangeType, ESuqsProgressViewDiffChangeType::Modified);
		TestEqual("First change should be modified task: questid", Diff.Entries[0].QuestID, FName("Q_Test1"));
		TestEqual("First change should be modified task: questid", Diff.Entries[0].TaskID, FName("T_1"));
	}
	Before = After;
	
	// Complete a task AND modify a quest description
	After.ActiveQuests[0].CurrentTasks[1].Status = ESuqsTaskStatus::Completed;
	After.ActiveQuests[0].Description = INVTEXT("Something new");
	TestTrue("Should generate changes", USuqsProgressViewHelpers::GetProgressViewDifferences(Before, After, Diff));
	if (TestEqual("Should be 2 changes", Diff.Entries.Num(), 2))
	{
		TestEqual("First change should be modified quest: category", Diff.Entries[0].Category, ESuqsProgressViewDiffCategory::Quest);
		TestEqual("First change should be modified quest: change", Diff.Entries[0].ChangeType, ESuqsProgressViewDiffChangeType::Modified);
		TestEqual("First change should be modified quest: questid", Diff.Entries[0].QuestID, FName("Q_Test1"));
	
		TestEqual("Second change should be completed Task: category", Diff.Entries[1].Category, ESuqsProgressViewDiffCategory::Task);
		TestEqual("Second change should be completed Task: change", Diff.Entries[1].ChangeType, ESuqsProgressViewDiffChangeType::Completed);
		TestEqual("Second change should be completed Task: questid", Diff.Entries[1].QuestID, FName("Q_Test1"));
		TestEqual("Second change should be completed Task: taskid", Diff.Entries[1].TaskID, FName("T_2"));
	
	}
	Before = After;
	
	// Fail 2 tasks and a quest
	After.ActiveQuests[0].CurrentTasks[0].Status = ESuqsTaskStatus::Failed;
	After.ActiveQuests[1].CurrentTasks[0].Status = ESuqsTaskStatus::Failed;
	After.ActiveQuests[0].Status = ESuqsQuestStatus::Failed;
	TestTrue("Should generate changes", USuqsProgressViewHelpers::GetProgressViewDifferences(Before, After, Diff));
	if (TestEqual("Should be 3 changes", Diff.Entries.Num(), 3))
	{
		TestEqual("First change should be failed quest: category", Diff.Entries[0].Category, ESuqsProgressViewDiffCategory::Quest);
		TestEqual("First change should be failed quest: change", Diff.Entries[0].ChangeType, ESuqsProgressViewDiffChangeType::Failed);
		TestEqual("First change should be failed quest: questid", Diff.Entries[0].QuestID, FName("Q_Test1"));
	
		TestEqual("Second change should be failed task: category", Diff.Entries[1].Category, ESuqsProgressViewDiffCategory::Task);
		TestEqual("Second change should be failed task: change", Diff.Entries[1].ChangeType, ESuqsProgressViewDiffChangeType::Failed);
		TestEqual("Second change should be failed task: questid", Diff.Entries[1].QuestID, FName("Q_Test1"));
		TestEqual("Second change should be failed task: taskid", Diff.Entries[1].TaskID, FName("T_1"));
	
		TestEqual("Third change should be failed task: category", Diff.Entries[2].Category, ESuqsProgressViewDiffCategory::Task);
		TestEqual("Third change should be failed task: change", Diff.Entries[2].ChangeType, ESuqsProgressViewDiffChangeType::Failed);
		TestEqual("Third change should be failed task: questid", Diff.Entries[2].QuestID, FName("Q_Test2"));
		TestEqual("Third change should be failed task: taskid", Diff.Entries[2].TaskID, FName("T_1"));
	}
	Before = After;

	// Remove a task and add a task
	{
		After.ActiveQuests[0].CurrentTasks.RemoveAt(0);
		auto& T1 = After.ActiveQuests[0].CurrentTasks.AddDefaulted_GetRef();
		T1.Identifier = "T_3";
		T1.Status = ESuqsTaskStatus::NotStarted;
		T1.bHidden = false;
		T1.Title = INVTEXT("A third thing");
	}
	TestTrue("Should generate changes", USuqsProgressViewHelpers::GetProgressViewDifferences(Before, After, Diff));
	if (TestEqual("Should be 2 changes", Diff.Entries.Num(), 2))
	{
		TestEqual("First change should be added task: category", Diff.Entries[0].Category, ESuqsProgressViewDiffCategory::Task);
		TestEqual("First change should be added task: change", Diff.Entries[0].ChangeType, ESuqsProgressViewDiffChangeType::Added);
		TestEqual("First change should be added task: questid", Diff.Entries[0].QuestID, FName("Q_Test1"));
		TestEqual("First change should be added task: questid", Diff.Entries[0].TaskID, FName("T_3"));
	
		TestEqual("Second change should be removed task: category", Diff.Entries[1].Category, ESuqsProgressViewDiffCategory::Task);
		TestEqual("Second change should be removed task: change", Diff.Entries[1].ChangeType, ESuqsProgressViewDiffChangeType::Removed);
		TestEqual("Second change should be removed task: questid", Diff.Entries[1].QuestID, FName("Q_Test1"));
		TestEqual("Second change should be removed task: taskid", Diff.Entries[1].TaskID, FName("T_1"));
	}
	Before = After;

	// Reset a quest, and complete a quest
	{
		After.ActiveQuests[0].Status = ESuqsQuestStatus::Incomplete;
		After.ActiveQuests[1].Status = ESuqsQuestStatus::Completed;
	}
	TestTrue("Should generate changes", USuqsProgressViewHelpers::GetProgressViewDifferences(Before, After, Diff));
	if (TestEqual("Should be 2 changes", Diff.Entries.Num(), 2))
	{
		TestEqual("First change should be reset quest: category", Diff.Entries[0].Category, ESuqsProgressViewDiffCategory::Quest);
		TestEqual("First change should be reset quest: change", Diff.Entries[0].ChangeType, ESuqsProgressViewDiffChangeType::Modified);
		TestEqual("First change should be reset quest: questid", Diff.Entries[0].QuestID, FName("Q_Test1"));
	
		TestEqual("Second change should be completed quest: category", Diff.Entries[1].Category, ESuqsProgressViewDiffCategory::Quest);
		TestEqual("Second change should be completed quest: change", Diff.Entries[1].ChangeType, ESuqsProgressViewDiffChangeType::Completed);
		TestEqual("Second change should be completed quest: questid", Diff.Entries[1].QuestID, FName("Q_Test2"));
	}
	Before = After;

	// Remove a quest
	After.ActiveQuests.RemoveAt(1);
	TestTrue("Should generate changes", USuqsProgressViewHelpers::GetProgressViewDifferences(Before, After, Diff));
	if (TestEqual("Should be 1 change", Diff.Entries.Num(), 1))
	{
		TestEqual("First change should be remove quest: category", Diff.Entries[0].Category, ESuqsProgressViewDiffCategory::Quest);
		TestEqual("First change should be remove quest: change", Diff.Entries[0].ChangeType, ESuqsProgressViewDiffChangeType::Removed);
		TestEqual("First change should be remove quest: questid", Diff.Entries[0].QuestID, FName("Q_Test2"));
	}
	Before = After;

	// Remove last quest
	After.ActiveQuests.RemoveAt(0);
	TestTrue("Should generate changes", USuqsProgressViewHelpers::GetProgressViewDifferences(Before, After, Diff));
	if (TestEqual("Should be 1 change", Diff.Entries.Num(), 1))
	{
		TestEqual("First change should be remove quest: category", Diff.Entries[0].Category, ESuqsProgressViewDiffCategory::Quest);
		TestEqual("First change should be remove quest: change", Diff.Entries[0].ChangeType, ESuqsProgressViewDiffChangeType::Removed);
		TestEqual("First change should be remove quest: questid", Diff.Entries[0].QuestID, FName("Q_Test1"));
	}
	Before = After;

	return true;
}
