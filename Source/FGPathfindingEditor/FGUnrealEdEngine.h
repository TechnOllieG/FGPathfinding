#pragma once
#include "Editor/UnrealEdEngine.h"
#include "FGGridFeatureVisualizer.h"
#include "FGUnrealEdEngine.generated.h"

UCLASS()
class FGPATHFINDINGEDITOR_API UFGUnrealEdEngine : public UUnrealEdEngine
{
	GENERATED_BODY()

public:
	virtual void Init(IEngineLoop* InEngineLoop) override;

private:
	TSharedPtr<FFGGridFeatureVisualizer> GridFeatureVisualizer;
};