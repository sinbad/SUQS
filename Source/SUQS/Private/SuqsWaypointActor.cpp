#include "SuqsWaypointActor.h"

#include "SuqsWaypointComponent.h"
#include "Components/WidgetComponent.h"


ASuqsWaypointActorBase::ASuqsWaypointActorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicatingMovement(true);

	const auto Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);
	WaypointComponent = CreateDefaultSubobject<USuqsWaypointComponent>("WaypointComponent");
	WaypointComponent->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);

	WaypointComponent->OnWaypointEnabledChanged.AddDynamic(
		this, &ASuqsWaypointActorBase::OnWaypointVisibilityPotentiallyChanged);
	WaypointComponent->OnWaypointIsCurrentChanged.AddDynamic(
		this, &ASuqsWaypointActorBase::OnWaypointVisibilityPotentiallyChanged);
}

void ASuqsWaypointActorBase::BeginPlay()
{
	Super::BeginPlay();

	UpdateWaypointVisibility();
}

void ASuqsWaypointActorBase::OnWaypointVisibilityPotentiallyChanged(USuqsWaypointComponent* Waypoint)
{
	UpdateWaypointVisibility();
}

void ASuqsWaypointActorBase::UpdateWaypointVisibility() const
{
	const bool bVisible = WaypointComponent->IsEnabled() && WaypointComponent->IsCurrent();
	UpdateWaypointWidget(bVisible);
}

ASuqsWaypointActor::ASuqsWaypointActor()
{
	VisualWidget = CreateDefaultSubobject<UWidgetComponent>("Visual");
	VisualWidget->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void ASuqsWaypointActor::UpdateWaypointWidget_Implementation(bool bVisible) const
{
	VisualWidget->SetVisibility(bVisible);
}
