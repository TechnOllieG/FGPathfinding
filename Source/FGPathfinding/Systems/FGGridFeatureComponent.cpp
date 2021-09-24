#include "FGGridFeatureComponent.h"

DEFINE_LOG_CATEGORY(LogGridFeatureComponent);

#if WITH_EDITOR
void UFGGridFeatureComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	UpdateGrid();
}

void UFGGridFeatureComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);

	UpdateGrid();
}
#endif

void UFGGridFeatureComponent::UpdateGrid()
{
	AFGGrid* Grid = GetOwner<AFGGrid>();

	if(Grid != nullptr)
	{
		Grid->GenerateGrid();
	}
	else
	{
		UE_LOG(LogGridFeatureComponent, Warning, TEXT("Grid feature component attached to actor with name: %s, the grid feature component should only be attached to FGGrid"), *GetOwner()->GetName());
	}
}