#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Internationalization/Text.h"
#include "SuqsParameterProvider.generated.h"

/// Convenience object to hold named parameters for compatibility with Blueprints and C++
UCLASS(BlueprintType)
class SUQS_API USuqsNamedFormatParams : public UObject
{
	GENERATED_BODY()
private:
	FFormatNamedArguments NamedArgs;

public:
	template <typename T>
	void SetParameter(const FString& Name, const T& Value) { NamedArgs.Add(Name, Value); }

	/// Set a text parameter
	UFUNCTION(BlueprintCallable)
	void SetTextParameter(FString Name, FText Value) { SetParameter(Name, Value); }
	/// Set an int parameter
	UFUNCTION(BlueprintCallable)
	void SetIntParameter(FString Name, int32 Value) { SetParameter(Name, Value); }
	/// Set an int64 parameter
	UFUNCTION(BlueprintCallable)
	void SetInt64Parameter(FString Name, int64 Value) { SetParameter(Name, Value); }
	/// Set a float parameter
	UFUNCTION(BlueprintCallable)
	void SetFloatParameter(FString Name, float Value) { SetParameter(Name, Value); }
	UFUNCTION(BlueprintCallable)
	/// Set a gender parameter
	void SetGenderParameter(FString Name, ETextGender Value) { SetParameter(Name, Value); }
	/// Set all parameters at once from a pre-prepared source
	UFUNCTION(BlueprintCallable)
	void SetAllParameters(const USuqsNamedFormatParams* SourceArgs) { NamedArgs.Empty(); NamedArgs.Append(SourceArgs->NamedArgs); }

	void Empty() { NamedArgs.Empty(); }
	FText Format(const FText& FormatText) const
	{
		return FText::Format(FormatText, NamedArgs);
	}
};

UINTERFACE()
class USuqsParameterProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for an object to fulfil if it wants to substitute parameter values into quest text.
 * You can include ordered or named parameters in quest text, and when they're detected, any requests for
 * that text will cause a request to be issued to all of the implementors of this interface which are registered
 * with the USuqsProgression instance.
 */
class SUQS_API ISuqsParameterProvider
{
	GENERATED_BODY()

public:
	
	/**
	 * Callback to provide named parameters for a given top-level Quest title or description. This callback will
	 * only be called if named parameters are needed. 
	 * @param QuestID The quest ID; will always be provided
	 * @param TaskID The task ID: may be None if this is for the root quest, or a task ID if it's for a specific task
	 * @param Params Use this object to set the named parameters you need
	 */
	UFUNCTION(BlueprintNativeEvent)
	void GetQuestParameters(const FName& QuestID, const FName& TaskID, USuqsNamedFormatParams* Params);
	
};
