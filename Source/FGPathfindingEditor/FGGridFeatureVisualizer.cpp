#include "FGGridFeatureVisualizer.h"

#include "FGPathfinding/Systems/FGGridFeatureComponent.h"

void FFGGridFeatureVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UFGGridFeatureComponent* GridFeatureComponent = Cast<UFGGridFeatureComponent>(Component);

	if(GridFeatureComponent == nullptr && GridFeatureComponent->GetOwner() == nullptr)
		return;

	FLinearColor Color;

	switch(GridFeatureComponent->Feature.GetValue())
	{
	case Obstacle:
		Color = FLinearColor::Red;
		break;
	default:
	case Standard:
		Color = FLinearColor::White;
		break;
	}

	const FMatrix Matrix = GridFeatureComponent->GetComponentTransform().ToMatrixNoScale();
	const FVector Extents = GridFeatureComponent->Extents;

	const FBox Box = FBox(-Extents, Extents);
	::DrawWireBox(PDI, Matrix, Box, Color, 0);
}
