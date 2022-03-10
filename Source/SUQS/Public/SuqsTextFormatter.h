#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SuqsTextFormatter.generated.h"

UINTERFACE()
class USuqsTextFormatter : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for an object to fulfil if it wants to substitute parameter values into quest text.
 * You can include ordered or named parameters in quest text, and when they're detected, any requests for
 * that text will cause a request to be issued to all of the implementors of this interface which are registered
 * with the USuqsProgression instance.
 */
class SUQS_API ISuqsTextFormatter
{
	GENERATED_BODY()

public:
	/**
	 * Optionally format quest title text to include parameter values. This is only called if the quest title has
	 * parameters (ordered or named) which need values.
	 * @param QuestID The identifier of the quest
	 * @param FormatText The original title text from the quest definition
	 * @param OutFormattedText The formatted text with completed parameters, which you may populate
	 * @return Return true if this implementation populated the text. False if it didn't and another text formatter
	 * should be given the opportunity to do so.
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool FormatQuestTitle(const FName& QuestID, const FText& InFormatText, FText& OutFormattedText);

	/**
	 * Optionally format quest description text to include parameter values. This is only called if the quest description has
	 * parameters (ordered or named) which need values.
	 * @param QuestID The identifier of the quest
	 * @param FormatText The original description text from the quest definition
	 * @param OutFormattedText The formatted text with completed parameters, which you may populate
	 * @return Return true if this implementation populated the text. False if it didn't and another text formatter
	 * should be given the opportunity to do so.
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool FormatQuestDescription(const FName& QuestID, const FText& InFormatText, FText& OutFormattedText);

	/**
	 * Optionally format task title text to include parameter values. This is only called if the task title has
	 * parameters (ordered or named) which need values.
	 * @param QuestID The identifier of the quest
	 * @param TaskID The identifier of the task
	 * @param FormatText The original description text from the quest definition
	 * @param OutFormattedText The formatted text with completed parameters, which you may populate
	 * @return Return true if this implementation populated the text. False if it didn't and another text formatter
	 * should be given the opportunity to do so.
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool FormatTaskTitle(const FName& QuestID, const FName& TaskID, const FText& InFormatText, FText& OutFormattedText);
	
	
};
