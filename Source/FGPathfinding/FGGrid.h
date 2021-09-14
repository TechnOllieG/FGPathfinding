#pragma once
#include "GameFramework/Actor.h"
#include "FGGrid.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGrid, Log, All);

UCLASS()
class FGPATHFINDING_API AFGGrid : public AActor
{
	GENERATED_BODY()

public:
	AFGGrid();
	virtual void BeginPlay() override;
	// When actor is placed in level, gets called again when transform is changed
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* GridLineComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* GridPlaneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int GridWidth = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int GridHeight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GridPointScale = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GridLineOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* PlaneMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterial* GridLineMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterial* GridPlaneMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FVector> GridPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor GridLineColor;

	/** The grid lines width in units */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GridLineWidth = 50.f;
	
#if WITH_EDITOR
	// When any properties is changed on this actor
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void GenerateGrid();

	UPROPERTY(Transient)
	AActor* GridLines;

	UPROPERTY(Transient)
	AActor* GridPlane;

	UPROPERTY()
	UMaterialInstanceDynamic* LineMaterial;
};
