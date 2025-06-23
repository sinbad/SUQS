// 

#pragma once

#include "CoreMinimal.h"
#include "SuqsProgression.h"
#include "SuqsProgressView.h"
#include "Components/ActorComponent.h"
#include "SuqsGameStateComponent.generated.h"

class USuqsProgression;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSuqsOnProgressViewChanged, USuqsGameStateComponent*, SuqsComp, const FSuqsProgressView&, Progress);
/**
 * Actor component that should be created on your GameState actor. You don't need to use this, you
 * can use USuqsProgression directly if you'd prefer. But this component is required if you intend
 * to use SUQS in multiplayer.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SUQS_API USuqsGameStateComponent : public UActorComponent
{
	GENERATED_BODY()
protected:
	/// Server-only progression class
	UPROPERTY()
	USuqsProgression* ServerProgression = nullptr;

	/// View on the current progress state, available everywhere
	UPROPERTY(ReplicatedUsing=OnRep_Progress)
	FSuqsProgressView ProgressView;
	
	UFUNCTION()
	void OnProgressionEvent(const FSuqsProgressionEventDetails& Details);

	void InitServerProgress();

public:
	/// Event is raised whenever quest progress changes
	/// Note that because replication is one-shot, you can't know which element has changed without
	/// comparing with previous state (which you are free to copy)
	UPROPERTY(BlueprintAssignable)
	FSuqsOnProgressViewChanged OnProgressChanged;
	
	USuqsGameStateComponent();

	virtual void BeginPlay() override;
	/// Return the SUQS progression object in order to perform server-side quest maintenance.
	/// You cannot call this from multiplayer clients!
	UFUNCTION(BlueprintCallable)
	USuqsProgression* GetServerProgression();

	/// Retrieve a view on the current progress state. This can be called on both servers and clients.
	UFUNCTION(BlueprintPure)
	const FSuqsProgressView& GetProgress() const { return ProgressView; }

	UFUNCTION()
	void OnRep_Progress();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
