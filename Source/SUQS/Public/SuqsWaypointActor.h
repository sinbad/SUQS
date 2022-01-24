#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SuqsWaypointActor.generated.h"

class UWidgetComponent;
class USuqsWaypointComponent;
/// Waypoint actor which brings together the waypoint component and a UMG visual
/// NOTE: if using a save/load system like SPUD you'll need a subclass with a post-restore hook to call UpdateWaypointVisibility()
UCLASS(Blueprintable, ClassGroup=(SUQS))
class SUQS_API ASuqsWaypointActor : public AActor
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	USuqsWaypointComponent* WaypointComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* VisualWidget;
public:
	ASuqsWaypointActor();


protected:
	UFUNCTION()
	virtual void OnWaypointVisibilityPotentiallyChanged(USuqsWaypointComponent* Waypoint);
	
	virtual void UpdateWaypointVisibility() const;
};
