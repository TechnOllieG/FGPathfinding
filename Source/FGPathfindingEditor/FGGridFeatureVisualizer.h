#pragma once
#include "ComponentVisualizer.h"

class FGPATHFINDINGEDITOR_API FFGGridFeatureVisualizer : public FComponentVisualizer
{
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};