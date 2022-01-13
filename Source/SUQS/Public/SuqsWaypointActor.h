#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SuqsWaypointActor.generated.h"

class UWidgetComponent;
class USuqsWaypointComponent;
/// Waypoint actor which brings together the waypoint component and a UMG visual
UCLASS(Blueprintable, ClassGroup=(SUQS))
class SUQS_API ASuqsWaypointActor : public AActor
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USuqsWaypointComponent* WaypointComponent;
	static constexpr int CurrentSavedVersion = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* VisualWidget;
	
public:
	ASuqsWaypointActor();


protected:
	UFUNCTION()
	virtual void OnWaypointVisibilityPotentiallyChanged(USuqsWaypointComponent* Waypoint);
	
	virtual void UpdateWaypointVisibility() const;
};
