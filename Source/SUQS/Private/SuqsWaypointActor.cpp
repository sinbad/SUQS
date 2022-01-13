#include "SuqsWaypointActor.h"

#include "SuqsWaypointComponent.h"
#include "Components/WidgetComponent.h"


ASuqsWaypointActor::ASuqsWaypointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	const auto Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);
	WaypointComponent = CreateDefaultSubobject<USuqsWaypointComponent>("WaypointComponent");
	WaypointComponent->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);

	VisualWidget = CreateDefaultSubobject<UWidgetComponent>("Visual");
	VisualWidget->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);

	WaypointComponent->OnWaypointEnabledChanged.AddDynamic(this, &ASuqsWaypointActor::OnWaypointVisibilityPotentiallyChanged);
	WaypointComponent->OnWaypointIsCurrentChanged.AddDynamic(this, &ASuqsWaypointActor::OnWaypointVisibilityPotentiallyChanged);

	UpdateWaypointVisibility();
}


void ASuqsWaypointActor::OnWaypointVisibilityPotentiallyChanged(USuqsWaypointComponent* Waypoint)
{
	UpdateWaypointVisibility();
}

void ASuqsWaypointActor::UpdateWaypointVisibility() const
{
	const bool bVisible = WaypointComponent->IsEnabled() && WaypointComponent->IsCurrent();
	VisualWidget->SetVisibility(bVisible);
}

