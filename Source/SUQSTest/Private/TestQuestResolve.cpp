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
