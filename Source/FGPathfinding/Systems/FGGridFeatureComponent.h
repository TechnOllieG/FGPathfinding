#pragma once
#include "Components/SceneComponent.h"
#include "FGGrid.h"
#include "FGGridFeatureComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGridFeatureComponent, Log, All);

UCLASS(meta=(BlueprintSpawnableComponent), BlueprintType)
class FGPATHFINDING_API UFGGridFeatureComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EGridFeature> Feature;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Extents;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditComponentMove(bool bFinished) override;
#endif

private:
	void UpdateGrid();
};