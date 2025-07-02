// 

#pragma once

#include "CoreMinimal.h"
#include "SuqsProgression.h"
#include "SuqsProgressView.h"
#include "Components/ActorComponent.h"
#include "SuqsGameStateComponent.generated.h"

class USuqsProgression;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSuqsOnProgressViewChanged, USuqsGameStateComponent*, SuqsComp, const FSuqsProgressView&, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSuqsOnProgressViewChangedWithDiff, USuqsGameStateComponent*, SuqsComp, const FSuqsProgressView&, ProgressSnapshot, const FSuqsProgressViewDiff&, ProgressDiff);
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

	bool bServerPendingChanges = false;

	/// View on the current progress state, available everywhere
	UPROPERTY(ReplicatedUsing=OnRep_Progress)
	FSuqsProgressView ProgressView;

	/// Whether to include completed/failed objectives in the progress view, rather than just the
	/// current objective. This will only include tasks which are not hidden, so use the "Always Visible"
	/// option on tasks as well to ensure they remain visible individually. This means more data but
	/// can be useful if you want to show completed tasks in previous objectives as well.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeCompletedObjectives = false;

	/// The previous progress snapshot, used to generate diffs
	FSuqsProgressView PreviousProgressView;
	FSuqsProgressViewDiff ProgressDiff;

	UFUNCTION()
	void OnProgressionEvent(const FSuqsProgressionEventDetails& Details);

	void InitServerProgress();
	void FireChangedEvent();

public:
	/// Event is raised whenever quest progress changes, and just supplies a snapshot of the current state.
	/// If you want a diff of the changes, bind to OnProgressChangedWithDiff instead
	UPROPERTY(BlueprintAssignable)
	FSuqsOnProgressViewChanged OnProgressChanged;

	/// Alternative event that is raised whenever quest progress changes, and supplies both a snapshot of the current state
	UPROPERTY(BlueprintAssignable)
	FSuqsOnProgressViewChangedWithDiff OnProgressChangedWithDiff;
	
	USuqsGameStateComponent();

	virtual void BeginPlay() override;
	/// Return the SUQS progression object in order to perform server-side quest maintenance.
	/// You cannot call this from multiplayer clients!
	UFUNCTION(BlueprintCallable)
	USuqsProgression* GetServerProgression();

	bool GetIncludeCompletedObjectives() const
	{
		return bIncludeCompletedObjectives;
	}

	/// Whether to include completed/failed objectives in the progress view, rather than just the
	/// current objective. This will only include tasks which are not hidden, so use the "Always Visible"
	/// option on tasks as well to ensure they remain visible individually. This means more data but
	/// can be useful if you want to show completed tasks in previous objectives as well.
	void SetIncludeCompletedObjectives(const bool bInclude)
	{
		bIncludeCompletedObjectives = bInclude;
	}

	/// Retrieve a view on the current progress state. This can be called on both servers and clients.
	UFUNCTION(BlueprintPure)
	const FSuqsProgressView& GetProgress() const { return ProgressView; }

	UFUNCTION()
	void OnRep_Progress();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
};
