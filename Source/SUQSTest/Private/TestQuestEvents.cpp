#include "Misc/AutomationTest.h"
#include "Engine.h"
#include "SuqsProgression.h"
#include "TestQuestData.h"
#include "CallbackCatcher.h"
#include "SuqsTaskState.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestTopLevelEvents, "SUQSTest.QuestTopLevelEvents",
                                 EAutomationTestFlags::EditorContext |
                                 EAutomationTestFlags::ClientContext |
                                 EAutomationTestFlags::ProductFilter)

bool FTestQuestTopLevelEvents::RunTest(const FString& Parameters)
{
	// This tests what happens with no branches enabled
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	UDataTable* QuestTable = NewObject<UDataTable>();
	QuestTable->RowStruct = FSuqsQuest::StaticStruct();
	QuestTable->bIgnoreMissingFields = true;
	QuestTable->CreateTableFromJSONString(SimpleMainQuestJson);
	Progression->QuestDataTables.Add(QuestTable);

	UCallbackCatcher* CallbackObj = NewObject<UCallbackCatcher>();

	// Add out event handlers
	Progression->OnQuestAccepted.AddDynamic(CallbackObj, &UCallbackCatcher::OnQuestAccepted);
	Progression->OnQuestFailed.AddDynamic(CallbackObj, &UCallbackCatcher::OnQuestFailed);
	Progression->OnQuestCompleted.AddDynamic(CallbackObj, &UCallbackCatcher::OnQuestCompleted);

	TestEqual("Should be no accepted to start", CallbackObj->AcceptedQuests.Num(), 0);
	TestTrue("Accept quest OK", Progression->AcceptQuest("Q_Main1"));
	TestEqual("Should have got one accept callback", CallbackObj->AcceptedQuests.Num(), 1);
	auto Q = Progression->GetQuest("Q_Main1");
	if (CallbackObj->AcceptedQuests.Num() > 0)
		TestEqual("Should have received correct quest callback", CallbackObj->AcceptedQuests[0], Q);

	TestEqual("Should be no failed to start", CallbackObj->FailedQuests.Num(), 0);
	Progression->GetNextMandatoryTask("Q_Main1")->Fail();
	TestEqual("Should be a failed callback", CallbackObj->FailedQuests.Num(), 1);
	if (CallbackObj->FailedQuests.Num() > 0)
		TestEqual("Should have received correct quest callback", CallbackObj->FailedQuests[0], Q);

	Q->Reset();
	TestEqual("Should have got another accept callback on reset", CallbackObj->AcceptedQuests.Num(), 2);
	if (CallbackObj->AcceptedQuests.Num() > 1)
		TestEqual("Should have received correct quest callback", CallbackObj->AcceptedQuests[1], Q);

	TestEqual("Should be no completed to start", CallbackObj->CompletedQuests.Num(), 0);
	Q->Complete();
	TestEqual("Should get quest completion callback", CallbackObj->CompletedQuests.Num(), 1);
	if (CallbackObj->CompletedQuests.Num() > 0)
		TestEqual("Should have received correct quest callback", CallbackObj->CompletedQuests[0], Q);
	
	return true;
}