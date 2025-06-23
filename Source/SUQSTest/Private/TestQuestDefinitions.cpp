#include "Misc/AutomationTest.h"
#include "Engine.h"
#include "SuqsProgression.h"
#include "TestQuestData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestCreateQuestDefinitions, "SUQSTest.CreateQuestDefinitions",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestCreateQuestDefinitions::RunTest(const FString& Parameters)
{
	UDataTable* QuestTable = USuqsProgression::MakeQuestDataTableFromJSON(SimpleMainQuestJson);

	if (!TestEqual("Should have 2 quests in table", QuestTable->GetRowMap().Num(), 2))
		return false;

	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestRuntimeQuestDefinitions, "SUQSTest.RuntimeQuestDefinitions",
								 EAutomationTestFlags::EditorContext |
								 EAutomationTestFlags::ClientContext |
								 EAutomationTestFlags::ProductFilter)

bool FTestRuntimeQuestDefinitions::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();

	// Minimal quest
	FSuqsQuest RTQuest;
	RTQuest.Identifier = "Q_Runtime";
	RTQuest.Title = INVTEXT("A simple runtime quest");
	FSuqsObjective& Obj = RTQuest.Objectives.Add_GetRef(FSuqsObjective());
	Obj.Identifier = "O_1";
	FSuqsTask& Task = Obj.Tasks.Add_GetRef(FSuqsTask());
	Task.Identifier = "T_SomeTask";
	Task.Title = INVTEXT("A Task");

	TestTrue("Runtime quest should be created", Progression->CreateQuestDefinition(RTQuest));
	TestFalse("Shouldn't be able to create quest twice", Progression->CreateQuestDefinition(RTQuest));

	TestTrue("Should be able to accept runtime quest", Progression->AcceptQuest("Q_Runtime"));

	TestTrue("Task should complete", Progression->CompleteTask("Q_Runtime", "T_SomeTask"));
	TestTrue("Quest should be complete", Progression->IsQuestCompleted("Q_Runtime"));
	TestTrue("Delete runtime quest should work", Progression->DeleteQuestDefinition("Q_Runtime"));
	TestNull("Quest should be gone", Progression->GetQuestDefinition("Q_Runtime"));

	FSuqsQuest EmptyQuest;
	AddExpectedError("Identifier is None", EAutomationExpectedMessageFlags::Contains, 1, false);
	TestFalse("Shouldn't be able to create a quest with no ID", Progression->CreateQuestDefinition(EmptyQuest));

	// Copy a template
	Progression->InitWithQuestDataTables(
		TArray<UDataTable*> {
			USuqsProgression::MakeQuestDataTableFromJSON(SimpleMainQuestJson)
		}
	);
	FSuqsQuest QuestCopy = Progression->GetQuestDefinitionCopy("Q_Main1");
	QuestCopy.Identifier = "Q_Main1_Copy";
	QuestCopy.Title = INVTEXT("A Copy");
	QuestCopy.Objectives[0].Tasks[0].Title = INVTEXT("A modified task");
	QuestCopy.Objectives[0].Tasks[2].TargetNumber = 10;
	TestTrue("Quest copy should be created", Progression->CreateQuestDefinition(QuestCopy));

	TestTrue("Should be able to accept quest copy", Progression->AcceptQuest("Q_Main1_Copy"));
	TArray<USuqsQuestState*> AcceptedQuests;
	Progression->GetAcceptedQuests(AcceptedQuests);
	TestEqual("Should be 1 quest active", AcceptedQuests.Num(), 1);
	TestEqual("Quest title should be modified", AcceptedQuests[0]->GetTitle().ToString(), "A Copy");
	TestEqual("Task title should be modified", AcceptedQuests[0]->GetCurrentObjective()->GetTasks()[0]->GetTitle().ToString(), "A modified task");
	TestEqual("Task target number should be modified", AcceptedQuests[0]->GetCurrentObjective()->GetTasks()[2]->GetTargetNumber(), 10);
	return true;
}
