#include "FGGrid.h"

#include "FGGridFeatureComponent.h"
#include "FGPathfinding/FGGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LogGrid);

AFGGrid::AFGGrid()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
	PrimaryActorTick.bCanEverTick = true;
}

void AFGGrid::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AFGGameState* GameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	if(GameState != nullptr)
		GameState->GridsInWorld.Add(this);
}

void AFGGrid::BeginPlay()
{
	Super::BeginPlay();

	if(PlaneMesh && GridPlaneMaterial)
		ResetPlaneTexture();
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

	BuildFeatureMapping();

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

	if(GridPlaneMaterial)
	{
		if(!GridPlaneComponent)
		{
			GridPlaneComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("GridPlaneComponent"));
			AddInstanceComponent(GridPlaneComponent);
			GridPlaneComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, false));
			GridPlaneComponent->SetCollisionProfileName(TEXT("NoCollision"));
			GridPlaneComponent->SetStaticMesh(PlaneMesh);
		}

		if(!PlaneMaterial)
			PlaneMaterial = GridPlaneComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, GridPlaneMaterial);
		
		ResetPlaneTexture();

		const float LengthX = GridWidth * GridPointScale;
		const float LengthY = GridHeight * GridPointScale;

		// Assumes the grid plane plane mesh has a size of 100x100 units at 1x1x1 scale.
		GridPlaneComponent->SetRelativeScale3D(FVector(LengthX * 0.01f, LengthY * 0.01f, 1.f));
	}
}

void AFGGrid::SetPixelsOnPlaneTexture(TArray<FColorIndexPair>& ColorData)
{
	if(!GridPlaneTexture)
	{
		CreatePlaneTexture();
	}
	
	FTexture2DMipMap& Mip = GridPlaneTexture->PlatformData->Mips[0];

	Mip.BulkData.Lock(LOCK_READ_WRITE);
	uint8* TextureData = static_cast<uint8*>(Mip.BulkData.Realloc(GridWidth * GridHeight * 4));
	for(int i = 0; i < ColorData.Num(); i++)
	{
		const int Index = ColorData[i].Index;
		const FColor Color = ColorData[i].Color;
		
		TextureData[4 * Index] = Color.B;
		TextureData[4 * Index + 1] = Color.G;
		TextureData[4 * Index + 2] = Color.R;
		TextureData[4 * Index + 3] = Color.A;
	}
	Mip.BulkData.Unlock();

	GridPlaneTexture->UpdateResource();
}

float AFGGrid::GetCostForGridIndex(int Index)
{
	return FeatureColorCostMap[FeatureMapping[Index]].CostMultiplier;
}

void AFGGrid::SetPixelOnPlaneTexture(int Index, FColor Color)
{
	TArray<FColorIndexPair> ColorData;
	ColorData.Add(FColorIndexPair(Index, Color));
	SetPixelsOnPlaneTexture(ColorData);
}

void AFGGrid::ResetPlaneTexture()
{
	CreatePlaneTexture();
	BuildFeatureMapping();
	
	PreviousFeatureColorCostMap = FeatureColorCostMap;
	
	uint8* Pixels = new uint8[GridWidth * GridHeight * 4];
	for (int y = 0; y < GridHeight; y++)
	{
		for (int x = 0; x < GridWidth; x++)
		{
			const int PixelIndex = ToGridIndex(FIntPoint(x, y));
			const EGridFeature Feature = FeatureMapping[PixelIndex];
			const FColorCostPair Pair = FeatureColorCostMap[Feature];
					
			Pixels[4 * PixelIndex] = Pair.Color.B;
			Pixels[4 * PixelIndex + 1] = Pair.Color.G;
			Pixels[4 * PixelIndex + 2] = Pair.Color.R;
			Pixels[4 * PixelIndex + 3] = Pair.Color.A;
		}
	}

	SetPlaneTexture(Pixels);

	delete[] Pixels;
}

FIntPoint AFGGrid::ToGridCoords(int Index)
{
	if (Index >= GridPoints.Num() || Index < 0)
		return FIntPoint(-1, -1);

	const FIntPoint Coordinate = FIntPoint(Index % GridWidth, Index / GridWidth);

	return Coordinate;
}

int AFGGrid::ToGridIndex(FIntPoint GridCoordinates)
{
	if (GridCoordinates.X >= GridWidth || GridCoordinates.Y >= GridHeight)
		return -1;
			
	const int Index = GridCoordinates.X + GridCoordinates.Y * GridWidth;

	if (Index >= GridPoints.Num() || Index < 0)
		return -1;
	
	return Index;
}

void AFGGrid::CreatePlaneTexture()
{
	if(GridPlaneTexture && GridPlaneTexture->GetSizeX() == GridWidth && GridPlaneTexture->GetSizeY() == GridHeight)
		return;
	
	GridPlaneTexture = UTexture2D::CreateTransient(GridWidth, GridHeight);
	GridPlaneTexture->LODGroup = TEXTUREGROUP_Pixels2D;

	PlaneMaterial->SetTextureParameterValue(TEXT("GridTexture"), GridPlaneTexture);
}

void AFGGrid::SetPlaneTexture(const uint8* Colors)
{
	FTexture2DMipMap& Mip = GridPlaneTexture->PlatformData->Mips[0];

	Mip.BulkData.Lock(LOCK_READ_WRITE);
	uint8* TextureData = static_cast<uint8*>(Mip.BulkData.Realloc(GridWidth * GridHeight * 4));
	FMemory::Memcpy(TextureData, Colors, sizeof(uint8) * GridWidth * GridHeight * 4);
	Mip.BulkData.Unlock();

	GridPlaneTexture->UpdateResource();
}

void AFGGrid::BuildFeatureMapping()
{
	FeatureMapping.Empty();

	for(int i = 0; i < GridPoints.Num(); i++)
	{
		FeatureMapping.Add(Standard);
	}

	for(int i = 0; i < GridFeatureInfo.Num(); i++)
	{
		const FGridFeatureInfo Current = GridFeatureInfo[i];

		SetFeatureBetweenGridCoords(Current.FromCoordinate, Current.ToCoordinate, Current.Feature);
	}

	TArray<UFGGridFeatureComponent*> FeatureComponents;
	GetComponents<UFGGridFeatureComponent>(FeatureComponents);

	for(UFGGridFeatureComponent* Comp : FeatureComponents)
	{
		const FMatrix Matrix = Comp->GetComponentTransform().ToMatrixNoScale();
		const FVector Extents = Comp->Extents;
		const FVector A = Matrix.TransformPosition(Extents);
		const FVector B = Matrix.TransformPosition(-Extents);

		FIntPoint From = ToGridCoords(GetClosestIndex(A));
		FIntPoint To = ToGridCoords(GetClosestIndex(B));
		
		SetFeatureBetweenGridCoords(From, To, Comp->Feature);
	}
}

void AFGGrid::SetFeatureBetweenGridCoords(FIntPoint A, FIntPoint B, EGridFeature Feature)
{
	int LowestX, LowestY, HighestX, HighestY;
	
	if(A.X < B.X)
	{
		LowestX = A.X;
		HighestX = B.X;
	}
	else
	{
		LowestX = B.X;
		HighestX = A.X;
	}

	if(A.Y < B.Y)
	{
		LowestY = A.Y;
		HighestY = B.Y;
	}
	else
	{
		LowestY = B.Y;
		HighestY = A.Y;
	}

	for(int x = LowestX; x <= HighestX; x++)
	{
		for(int y = LowestY; y <= HighestY; y++)
		{
			const int Index = ToGridIndex(FIntPoint(x, y));
			FeatureMapping[Index] = Feature;
		}
	}
}

bool AFGGrid::MapsAreEqual(TMap<TEnumAsByte<EGridFeature>, FColorCostPair>& A, TMap<TEnumAsByte<EGridFeature>, FColorCostPair>& B)
{
	if(A.Num() != B.Num())
		return false;

	TArray<TEnumAsByte<EGridFeature>> Keys;
	FeatureColorCostMap.GetKeys(Keys);
	
	for(int i = 0; i < Keys.Num(); i++)
	{
		const EGridFeature Key = Keys[i];
		
		if(A[Key].Color != B[Key].Color)
			return false;
	}
	return true;
}


void AFGGrid::DebugDrawGridIndices(UObject* WorldContextObject)
{
	if(WorldContextObject == nullptr)
		WorldContextObject = this;

	UKismetSystemLibrary::DrawDebugString(WorldContextObject, FVector(0.f, 0.f, 0.f), TEXT("Test"), nullptr, DrawDebugColor);
	
	for(int i = 0; i < GridPoints.Num(); i++)
	{
		UKismetSystemLibrary::DrawDebugString(WorldContextObject, GridPoints[i], FString::FromInt(i), nullptr, DrawDebugColor);
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
