#include "SuqsTestParamProvider.h"

void USuqsTestParamProvider::GetQuestParameters_Implementation(const FName& QuestID,
	const FName& TaskID,
	USuqsNamedFormatParams* Params)
{
	++NumberOfTimesCalled;
	Params->SetTextParameter("TextParam", TextValue);
	Params->SetIntParameter("IntParam", IntValue);
	Params->SetInt64Parameter("Int64Param", Int64Value);
	Params->SetFloatParameter("FloatParam", FloatValue);
	Params->SetGenderParameter("GenderParam", GenderValue);
}

