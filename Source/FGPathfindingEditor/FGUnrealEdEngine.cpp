#include "FGUnrealEdEngine.h"

#include "FGGridFeatureVisualizer.h"
#include "FGPathfinding/Systems/FGGridFeatureComponent.h"

void UFGUnrealEdEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);

	GridFeatureVisualizer = MakeShareable(new FFGGridFeatureVisualizer);
	RegisterComponentVisualizer(UFGGridFeatureComponent::StaticClass()->GetFName(), GridFeatureVisualizer);
}
