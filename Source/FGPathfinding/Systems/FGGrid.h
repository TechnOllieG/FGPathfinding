#pragma once
#include "GameFramework/Actor.h"
#include "FGGrid.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGrid, Log, All);

USTRUCT(BlueprintType, Blueprintable)
struct FColorIndexPair
{
	GENERATED_BODY()

	FColorIndexPair() {}
	FColorIndexPair(int Index, FColor Color)
	{
		this->Index = Index;
		this->Color = Color;
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Index;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FColor Color;
};

UENUM()
enum EGridFeature
{
	Standard,
	Obstacle
};

USTRUCT(BlueprintType, Blueprintable)
struct FGridFeatureInfo
{
	GENERATED_BODY()

	FGridFeatureInfo() {}
	FGridFeatureInfo(FIntPoint FromCoordinate, FIntPoint ToCoordinate, EGridFeature Feature)
	{
		this->FromCoordinate = FromCoordinate;
		this->ToCoordinate = ToCoordinate;
		this->Feature = Feature;
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FIntPoint FromCoordinate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FIntPoint ToCoordinate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EGridFeature> Feature;
};

USTRUCT(BlueprintType, Blueprintable)
struct FColorCostPair
{
	GENERATED_BODY()

	FColorCostPair() {}
	FColorCostPair(FColor Color, float CostMultiplier)
	{
		this->Color = Color;
		this->CostMultiplier = CostMultiplier;
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FColor Color;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CostMultiplier;
};

UCLASS()
class FGPATHFINDING_API AFGGrid : public AActor
{
	GENERATED_BODY()

public:
	AFGGrid();
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	// When actor is placed in level, gets called again when transform is changed
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

#if WITH_EDITOR
	// When any properties is changed on this actor
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable)
	void GenerateGrid();

	UFUNCTION(BlueprintCallable)
	void DebugDrawGridIndices(UObject* WorldContextObject = nullptr);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* GridLineComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* GridPlaneComponent;

	/** A way to specify feature info on the grid without adding components (this was used mostly before gridfeaturecomponents existed) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FGridFeatureInfo> GridFeatureInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int GridWidth = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int GridHeight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GridPointScale = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GridLineOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool DrawDebugIndices = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor DrawDebugColor;

	/** This is the plane mesh that the grid line/grid plane material will be applied to */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* PlaneMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterial* GridLineMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterial* GridPlaneMaterial;

	/** The definition of what the color and cost of each feature should be */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<TEnumAsByte<EGridFeature>, FColorCostPair> FeatureColorCostMap = {{Standard, FColorCostPair(FColor::White, 1.f)}, {Obstacle, FColorCostPair(FColor::Red, -1.f)}};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FVector> GridPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor GridLineColor;

	/** The grid lines width in units */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GridLineWidth = 50.f;

	/** Gets neighbors to the input grid point. Diagonals can be given if wanted, -1 in the output array = there is no valid neighbor in that direction */
	UFUNCTION(BlueprintCallable)
	void GetNeighbors(int CurrentGridIndex, TArray<int>& OutNeighbors, bool IncludeDiagonals = true);

	/** Gets the closest grid index to a given world location */
	UFUNCTION(BlueprintCallable)
	int GetClosestIndex(FVector WorldLocation);

	/** Checks if grid array is valid with the current settings for the grid, can optionally regenerate the grid if not */
	UFUNCTION(BlueprintCallable)
	bool IsGridGenerationValid(bool RegenerateGridIfNot = false);

	UFUNCTION(BlueprintCallable)
	void SetPixelsOnPlaneTexture(TArray<FColorIndexPair>& ColorData);

	UFUNCTION(BlueprintCallable)
	float GetCostForGridIndex(int Index);
	
	UFUNCTION(BlueprintCallable)
	void SetPixelOnPlaneTexture(int Index, FColor Color);

	/** Fill plane texture with base color */
	UFUNCTION(BlueprintCallable)
	void ResetPlaneTexture();
	
	UFUNCTION(BlueprintCallable)
	FIntPoint ToGridCoords(int Index);

	UFUNCTION(BlueprintCallable)
	int ToGridIndex(FIntPoint GridCoordinates);

private:
	/** Will return the middle index of the grid (if the width/length of the grid is even it will return the top right middle when looking straight at the grid with grid point with index 0 in the bottom left */
	int GetMiddleIndex();
	
	/** Will check if the input index is valid, if not, will return -1 */
	int CheckValidIndex(int Input) const;

	/** If input is a valid index, will return true */
	bool IsValidIndex(int Input) const;

	void CreatePlaneTexture();
	void SetPlaneTexture(const uint8* Colors);
	void BuildFeatureMapping();
	void SetFeatureBetweenGridCoords(FIntPoint A, FIntPoint B, EGridFeature Feature);

	bool MapsAreEqual(TMap<TEnumAsByte<EGridFeature>, FColorCostPair>& A, TMap<TEnumAsByte<EGridFeature>, FColorCostPair>& B);

	UPROPERTY()
	TArray<TEnumAsByte<EGridFeature>> FeatureMapping;

	UPROPERTY(Transient)
	AActor* GridLines;

	UPROPERTY(Transient)
	UTexture2D* GridPlaneTexture;

	UPROPERTY()
	TMap<TEnumAsByte<EGridFeature>, FColorCostPair> PreviousFeatureColorCostMap;

	UPROPERTY()
	UMaterialInstanceDynamic* LineMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* PlaneMaterial;
};
