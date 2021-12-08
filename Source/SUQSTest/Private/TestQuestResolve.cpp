#include "Misc/AutomationTest.h"
#include "Engine.h"
#include "SuqsProgression.h"
#include "TestQuestData.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestTimedResolve,
                                 "SUQSTest.QuestTimedResolve",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestTimedResolve::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();

	Progression->InitWithQuestDataTables(
		TArray<UDataTable*> {
			USuqsProgression::MakeQuestDataTableFromJSON(NonAutoResolveQuestsJson)
		}
	);

	TestTrue("Accept timed resolve quest", Progression->AcceptQuest("Q_TimedResolve"));
	TestTrue("Complete task", Progression->CompleteTask("Q_TimedResolve", "T_Single"));

	// Because task is set to resolve on a time delay, shouldn't have resolved to complete quest yet
	TestFalse("Quest shouldn't be complete yet", Progression->IsQuestCompleted("Q_TimedResolve"));
	// Add a little time (should take 3s to complete)
	Progression->Tick(2.2);
	TestFalse("Quest shouldn't be complete after less than 3 seconds", Progression->IsQuestCompleted("Q_TimedResolve"));
	Progression->Tick(1.1);
	TestTrue("Quest should now be complete after 3 seconds", Progression->IsQuestCompleted("Q_TimedResolve"));

	TestTrue("Quest has its own further time delay before archived", Progression->IsQuestActive("Q_TimedResolve"));
	Progression->Tick(1);
	TestFalse("Quest should now be archived", Progression->IsQuestActive("Q_TimedResolve"));
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestGatedResolve,
								 "SUQSTest.QuestGatedResolve",
								 EAutomationTestFlags::EditorContext |
								 EAutomationTestFlags::ClientContext |
								 EAutomationTestFlags::ProductFilter)

bool FTestQuestGatedResolve::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();

	Progression->InitWithQuestDataTables(
		TArray<UDataTable*> {
			USuqsProgression::MakeQuestDataTableFromJSON(NonAutoResolveQuestsJson)
		}
	);

	TestTrue("Accept timed resolve quest", Progression->AcceptQuest("Q_GatedResolve"));
	TestTrue("Complete task", Progression->CompleteTask("Q_GatedResolve", "T_Single"));

	// Because task is set to resolve on a gate, shouldn't have resolved to complete quest yet
	TestFalse("Quest shouldn't be complete yet", Progression->IsQuestCompleted("Q_GatedResolve"));
	// Open gate: this should let through both task AND quest
	Progression->SetGateOpen("TestGate", true);
	TestTrue("Quest should now be complete after gate opened", Progression->IsQuestCompleted("Q_GatedResolve"));
	TestFalse("Quest should now be archived", Progression->IsQuestActive("Q_GatedResolve"));
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestManualResolve,
								 "SUQSTest.QuestManualResolve",
								 EAutomationTestFlags::EditorContext |
								 EAutomationTestFlags::ClientContext |
								 EAutomationTestFlags::ProductFilter)

bool FTestQuestManualResolve::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();

	Progression->InitWithQuestDataTables(
		TArray<UDataTable*> {
			USuqsProgression::MakeQuestDataTableFromJSON(NonAutoResolveQuestsJson)
		}
	);

	TestTrue("Accept timed resolve quest", Progression->AcceptQuest("Q_ManualResolve"));
	TestTrue("Complete task", Progression->CompleteTask("Q_ManualResolve", "T_Single"));

	// Because task is set to resolve manually, shouldn't have resolved to complete quest yet
	TestFalse("Quest shouldn't be complete yet", Progression->IsQuestCompleted("Q_ManualResolve"));
	// Resolve task
	Progression->ResolveTask("Q_ManualResolve", "T_Single");
	TestTrue("Quest should now be complete after task resolved", Progression->IsQuestCompleted("Q_ManualResolve"));
	
	TestTrue("Quest has its own further manual resolve", Progression->IsQuestActive("Q_ManualResolve"));
	Progression->ResolveQuest("Q_ManualResolve");
	TestFalse("Quest should now be archived", Progression->IsQuestActive("Q_ManualResolve"));
	
	return true;
}
