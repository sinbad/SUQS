# Saving Quest progression

Being able to [progress quests](Progression.md) is fine, but you also need to
save this progress along with your save games.

The `USuqsProgression` class performs its own serialisation, you can use its
`Serialize` method to save or load data to an archive of your choice.

## USaveGame Serialisation

If you're using the `USaveGame` approach to saving your game data, then you will
need to override the `Serialize` method to call `USuqsProgression::Serialize` as
well. 

You do need to call `Serialize`, you can't just put `USuqsProgression` in a
`UPROPERTY(SaveGame)`, because of the specific setup tasks needed. Here's an
example:

```c++

void UYourSaveGame::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);

    QuestProgression->Serialize(Ar);
}

```

This assumes that `QuestProgression` is a pointer to your `USuqsProgression`
object; where this is doesn't matter so long as you can reference it.

## SPUD (Steve's Persistent Unreal Data)

[SPUD](https://github.com/sinbad/SPUD) is one of my other libraries for handling
persistent world data. Like with `USaveGame`, the quest data held in 
`USuqsProgression` is not suitable for just storing in a `UPROPERTY(SaveGame)`,
so you have to call `Serialize`.

> I *could* actually make `USuqsProgression` work directly with SPUD, but it
> would require making SUQS dependent on SPUD to get access to the callbacks, 
> and I didn't want to do that.

You could just implement `ISpudObjectCallback` on any of your existing stored
global objects in SPUD and add a call to `USuqsProgression::Serialize`, but
I think the best way to do this is to make a small wrapper class which ties SUQS and SPUD
together. It's a little more verbose but keeps everything together. Here's
one you can use as-is if you like:

SuqsSpudWrapper.h:
```c++
#pragma once

#include "CoreMinimal.h"

#include "ISpudObject.h"
#include "SuqsProgression.h"
#include "UObject/Object.h"
#include "SuqsSpudWrapper.generated.h"

/**
 * A bridging class which makes it easy to save SUQS data in a SPUD save game.
 * This avoids making SUQS dependent on SPUD, use this wrapper if you use both.
 * 1. Store an instance of this wrapper in a UPROPERTY and call Init(), passing 
 *    the USuqsProgression instance.
 * 2. Then, call SPUD's AddPersistentGlobalObject to track this object. 
 *
 * Whenever SPUD saves / loads, this wrapper will process the SUQS progression 
 * data along with it.
 * 
 */
UCLASS()
class YOUR_API USuqsSpudWrapper : public UObject, public ISpudObjectCallback
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	USuqsProgression* Progression;
public:

	/// Initialise this wrappe with the progression object it will be tracking.
	void Init(USuqsProgression* InProgression);
	
	virtual void SpudStoreCustomData_Implementation(const USpudState* State, USpudStateCustomData* CustomData) override;
	virtual void SpudRestoreCustomData_Implementation(USpudState* State, USpudStateCustomData* CustomData) override;
};
```
SuqsSpudWrapper.cpp:
```c++
#include "SuqsSpudWrapper.h"

void USuqsSpudWrapper::Init(USuqsProgression* InProgression)
{
	Progression = InProgression;
}

void USuqsSpudWrapper::SpudStoreCustomData_Implementation(const USpudState* State, USpudStateCustomData* CustomData)
{
	if (Progression)
	{
		Progression->Serialize(*CustomData->GetUnderlyingArchive());
	}
	
}

void USuqsSpudWrapper::SpudRestoreCustomData_Implementation(USpudState* State, USpudStateCustomData* CustomData)
{
	if (Progression)
	{
		Progression->Serialize(*CustomData->GetUnderlyingArchive());
	}
}

```

To set this up, you need an instance that always exists (e.g. GameInstance) to 
hold both your `USuqsProgression` and `USuqsSpudWrapper` instances:

YourGameInstance.h:
```c++
UCLASS()
class YOUR_API UYourGameInstance : public UGameInstance
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(BlueprintReadOnly, SaveGame)
	USuqsProgression* QuestProgression;

	UPROPERTY()
	USuqsSpudWrapper* QuestSpudWrapper;

public:
	virtual void Init() override;
};
```

Then in your initialisation phase (GameInstance has an `Init` function which I
overrode above), you need to hook them up:

YourGameInstance.cpp:
```c++
void USnukaGameInstance::Init()
{
	Super::Init();

    // Create quest progression
	QuestProgression = NewObject<USuqsProgression>();
    // Init using quest assets - multiple options here, I'm using a path
	QuestProgression->InitWithQuestDataTablesInPath(QuestDataPath);

    // Now init the wrapper which talks to SPUD
	QuestSpudWrapper = NewObject<USuqsSpudWrapper>();
	QuestSpudWrapper->Init(QuestProgression);

    // Tell SPUD to store the wrapper, which will store the quest data too
	auto Spud = GetSpudSubsystem(GetWorld()); 
	Spud->AddPersistentGlobalObjectWithName(QuestSpudWrapper, "QuestProgression");
    // You might also want to add the GI itself as a SPUD global object ofc

}
```

After that you don't have to think about it, every time you save/load a game in 
SPUD, the quest progression will go along with it.