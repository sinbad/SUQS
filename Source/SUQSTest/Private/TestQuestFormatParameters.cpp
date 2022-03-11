#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "SuqsProgression.h"
#include "SuqsTestParamProvider.h"


#define LOCTEXT_NAMESPACE "TestQuests"


// Just using generic parameter names so it's easy to test all types
const FString QuestsWithParamsJson = R"RAWJSON([
	{
		"Identifier": "Q1",
		"Title": "NSLOCTEXT(\"TestQuests\", \"QuestWithNamedParamsTitle\", \"Meet {TextParam} in {IntParam} Days\")",
		"DescriptionWhenActive": "NSLOCTEXT(\"TestQuests\", \"QuestWithNamedParamsDesc\", \"Remember that {TextParam}'s favourite number is {Int64Param} within {FloatParam}\")",
		"Objectives": [
			{
				"Identifier": "O1",
				"Tasks": [
					{
						"Identifier": "T1Text",
						"Title": "NSLOCTEXT(\"TestQuests\", \"TTextParamDesc\", \"Go to {TextParam} and {TextParam}\")"
					},
					{
						"Identifier": "T2Ints",
						"Title": "NSLOCTEXT(\"TestQuests\", \"TIntParamsDesc\", \"{IntParam} to {Int64Param}\")"
					},
					{
						"Identifier": "T2FloatAndGender",
						"Title": "NSLOCTEXT(\"TestQuests\", \"TFloatGenderParamsDesc\", \"The number {FloatParam} is {GenderParam}|gender(masculine,feminine)\")"
					}
				]
			}
		]
	}
])RAWJSON";


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestQuestFormatParams, "SUQSTest.QuestFormatParams",
								 EAutomationTestFlags::EditorContext |
								 EAutomationTestFlags::ClientContext |
								 EAutomationTestFlags::ProductFilter)

bool FTestQuestFormatParams::RunTest(const FString& Parameters)
{
	USuqsProgression* Progression = NewObject<USuqsProgression>();
	Progression->InitWithQuestDataTables(
		TArray<UDataTable*> {
			USuqsProgression::MakeQuestDataTableFromJSON(QuestsWithParamsJson)
		}
	);

	auto Provider = NewObject<USuqsTestParamProvider>();
	Provider->TextValue = LOCTEXT("Steve", "Steve");
	Provider->IntValue = 12345;
	Provider->Int64Value = -9223372036854775800;
	Provider->FloatValue = 3.142f;
	Provider->GenderValue = ETextGender::Feminine;
	
	Progression->AddParameterProvider(Provider);

	Progression->AcceptQuest("Q1");
	auto Q1 = Progression->GetQuest("Q1");
	FText QuestTitle = Q1->GetTitle();
	FText QuestDesc = Q1->GetDescription();
	
	TestTrue("Quest Title should include params", QuestTitle.EqualTo(
		LOCTEXT("Q1Title", "Meet Steve in 12,345 Days")));
	TestTrue("Quest Description should include params", QuestDesc.EqualTo(
		LOCTEXT("Q1Desc", "Remember that Steve's favourite number is -9,223,372,036,854,775,800 within 3.142")));

	auto T1 = Q1->GetTask("T1Text");
	FText TaskTitle = T1->GetTitle();
	TestTrue("Task 1 Title should include params", TaskTitle.EqualTo(
		LOCTEXT("T1Title", "Go to Steve and Steve")));
	auto T2 = Q1->GetTask("T2Ints");
	TaskTitle = T2->GetTitle();
	TestTrue("Task 2 Title should include params", TaskTitle.EqualTo(
		LOCTEXT("T2Title", "12,345 to -9,223,372,036,854,775,800")));
	auto T3 = Q1->GetTask("T2FloatAndGender");
	TaskTitle = T3->GetTitle();
	TestTrue("Task 3 Title should include params", TaskTitle.EqualTo(
		LOCTEXT("T2Title", "The number 3.142 is feminine")));

	return true;
}