#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestAccept, "SUQSTest.QuestAccept",
	EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::ClientContext |
    EAutomationTestFlags::ProductFilter)

bool FTestQuestAccept::RunTest(const FString& Parameters)
{
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestReset, "SUQSTest.QuestReset",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::ClientContext |
    EAutomationTestFlags::ProductFilter)

bool FTestQuestReset::RunTest(const FString& Parameters)
{
	return true;
}
