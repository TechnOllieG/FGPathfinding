#include "FGGrid.h"

#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LogGrid);

AFGGrid::AFGGrid()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
	PrimaryActorTick.bCanEverTick = true;
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

void AFGGrid::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(DrawDebugIndices)
		DebugDrawGridIndices();
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
	
	for(int y = 0; y < GridHeight; y++)
	{
		for(int x = 0; x < GridWidth; x++)
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
			GridLineComponent->SetStaticMesh(PlaneMesh);
		}

		if(!LineMaterial)
			LineMaterial = GridLineComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, GridLineMaterial);
		
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
}

void AFGGrid::DebugDrawGridIndices()
{
	for(int i = 0; i < GridPoints.Num(); i++)
	{
		UKismetSystemLibrary::DrawDebugString(this, GridPoints[i], FString::FromInt(i), nullptr, DrawDebugColor);
	}
}

int AFGGrid::GetMiddleIndex()
{
	int HalfWidth = GridWidth * 0.5f;
	int HalfHeight = GridHeight * 0.5f;
			
	return HalfWidth + HalfHeight * GridWidth;
}

int AFGGrid::CheckValidIndex(int Input) const
{
	if(Input < 0 || Input >= GridPoints.Num())
		return -1;
	
	return Input;
}

bool AFGGrid::IsValidIndex(int Input) const
{
	return CheckValidIndex(Input) >= 0;
}

bool AFGGrid::IsGridGenerationValid(bool RegenerateGridIfNot)
{
	return GridPoints.Num() == GridWidth * GridHeight && GridPoints.Num() > 1 ? FMath::IsNearlyEqual(FVector::DistSquared(GridPoints[0], GridPoints[1]), FMath::Square(GridPointScale)) : true;
}

void AFGGrid::GetNeighbors(int CurrentGridIndex, TArray<int>& OutNeighbors, bool IncludeDiagonals)
{
	const int Num = GridPoints.Num();

	if(!IsValidIndex(CurrentGridIndex))
	{
		UE_LOG(LogGrid, Warning, TEXT("Couldn't get neighbors since current index is invalid"));
		return;
	}
	
	OutNeighbors.Add(CheckValidIndex(CurrentGridIndex - GridWidth));
	OutNeighbors.Add(CheckValidIndex(CurrentGridIndex + GridWidth));

	int Current = CurrentGridIndex;
	for(int i = 0; i < 3; i++)
	{
		if(Current >= 0)
		{
			const int Mod = Current % GridWidth;
			
			OutNeighbors.Add(Mod != GridWidth - 1 ? Current + 1 : -1);
			OutNeighbors.Add(Mod != 0 ? Current - 1 : -1);
		}
		
		if(!IncludeDiagonals)
			break;
		
		Current = OutNeighbors[i];
	}
}

int AFGGrid::GetClosestIndex(FVector WorldLocation)
{
	const int MiddleIndex = GetMiddleIndex();
	const int HalfWidth = GridWidth * 0.5f;
	const int HalfHeight = GridHeight * 0.5f;
	
	const FVector LocalPos = GetTransform().InverseTransformPosition(WorldLocation);
	const FVector2D FlattenedLocalPos = FVector2D(LocalPos.X, LocalPos.Y);
	
	const FVector2D FlattenedGridCoordinates = FlattenedLocalPos / GridPointScale;

	FIntPoint IndexOffsetFromCenter = FIntPoint();
	if(GridWidth % 2 == 0)
		IndexOffsetFromCenter.X = FMath::Floor(FlattenedGridCoordinates.X);
	else
		IndexOffsetFromCenter.X = FMath::RoundHalfFromZero(FlattenedGridCoordinates.X);

	if(GridHeight % 2 == 0)
		IndexOffsetFromCenter.Y = FMath::Floor(FlattenedGridCoordinates.Y);
	else
		IndexOffsetFromCenter.Y = FMath::RoundHalfFromZero(FlattenedGridCoordinates.Y);

	IndexOffsetFromCenter.X = FMath::Clamp(IndexOffsetFromCenter.X, -HalfWidth, HalfWidth - (GridWidth % 2 == 0));
	IndexOffsetFromCenter.Y = FMath::Clamp(IndexOffsetFromCenter.Y, -HalfHeight, HalfHeight - (GridHeight % 2 == 0));

	return MiddleIndex + IndexOffsetFromCenter.X + IndexOffsetFromCenter.Y * GridWidth;
}
