#include "FGGrid.h"

DEFINE_LOG_CATEGORY(LogGrid);

AFGGrid::AFGGrid()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
}

void AFGGrid::BeginPlay()
{
	Super::BeginPlay();
}

void AFGGrid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	GenerateGrid();
}

#if WITH_EDITOR
void AFGGrid::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	GenerateGrid();
}
#endif

void AFGGrid::GenerateGrid()
{
	if(GridWidth <= 0 || GridHeight <= 0 || GridPointScale <= 0.f)
	{
		UE_LOG(LogGrid, Warning, TEXT("Grid Width/Grid Height/Grid Point Scale is <= 0 and therefore the grid cannot generate"));
		return;
	}
	
	if(GridPoints.Num() > 0)
		GridPoints.Empty();

	GridPoints.Reserve(GridWidth * GridHeight);

	FVector StartPosition = -FVector(FMath::Floor(GridWidth * 0.5f) * GridPointScale, FMath::Floor(GridHeight * 0.5f) * GridPointScale, 0.f);
	
	if(GridWidth % 2 == 0)
		StartPosition += FVector(0.5f * GridPointScale, 0.f, 0.f);

	if(GridHeight % 2 == 0)
		StartPosition += FVector(0.f, 0.5f * GridPointScale, 0.f);
	
	for(int x = 0; x < GridWidth; x++)
	{
		for(int y = 0; y < GridHeight; y++)
		{
			FVector LocalPos = StartPosition + FVector(x * GridPointScale, y * GridPointScale, 0.f);
			GridPoints.Add(GetActorTransform().TransformPosition(LocalPos));
		}
	}

	if(!PlaneMesh)
		return;

	if(GridLineMaterial)
	{
		if(!GridLineComponent)
		{
			GridLineComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("GridLineComponent"));
			AddInstanceComponent(GridLineComponent);
			GridLineComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, false));
			GridLineComponent->SetCollisionProfileName(TEXT("NoCollision"));
		}
		
		LineMaterial = GridLineComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, GridLineMaterial);
		
		GridLineComponent->SetStaticMesh(PlaneMesh);
		GridLineComponent->SetRelativeLocation(FVector(0.f, 0.f, GridLineOffset));
		
		const float LengthX = GridWidth * GridPointScale + GridLineWidth * 2.f;
		const float LengthY = GridHeight * GridPointScale + GridLineWidth * 2.f;
		
		// Assumes the grid line plane mesh has a size of 100x100 units at 1x1x1 scale.
		GridLineComponent->SetRelativeScale3D(FVector(LengthX * 0.01f, LengthY * 0.01f, 1.f));
		
		LineMaterial->SetScalarParameterValue(TEXT("GridWidth"), GridWidth);
		LineMaterial->SetScalarParameterValue(TEXT("GridHeight"), GridHeight);
		LineMaterial->SetVectorParameterValue(TEXT("LineColor"), GridLineColor);

		float InputLineWidth;
		if(LengthX > LengthY)
			InputLineWidth = GridLineWidth / LengthY;
		else
			InputLineWidth = GridLineWidth / LengthX;
			
		LineMaterial->SetScalarParameterValue(TEXT("LineWidth"), InputLineWidth);
	}

	if(GridPlaneMaterial)
	{
		
	}
}
