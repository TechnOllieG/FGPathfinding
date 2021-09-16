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
	return;

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
		
		if(!GridPlaneTexture || GridPlaneTexture->GetSizeX() != GridWidth || GridPlaneTexture->GetSizeY() != GridHeight)
		{
			GridPlaneTexture = UTexture2D::CreateTransient(GridWidth, GridHeight);

			uint8* Pixels = new uint8[GridWidth * GridHeight * 4];
			for (int y = 0; y < GridHeight; y++)
			{
				for (int x = 0; x < GridWidth; x++)
				{
					int PixelIndex = y * GridWidth + x;
					Pixels[4 * PixelIndex] = PixelIndex % 2 == 0 * 255;
					Pixels[4 * PixelIndex + 1] = PixelIndex % 2 == 0 * 255;
					Pixels[4 * PixelIndex + 2] = PixelIndex % 2 == 0 * 255;
					Pixels[4 * PixelIndex + 3] = 255;
				}
			}

			FTexture2DMipMap& Mip = GridPlaneTexture->PlatformData->Mips[0];
			
			Mip.BulkData.Lock(LOCK_READ_WRITE);
			uint8* TextureData = (uint8*) Mip.BulkData.Realloc(GridWidth * GridHeight * 4);
			FMemory::Memcpy(TextureData, Pixels, sizeof(uint8) * GridWidth * GridHeight * 4);
			Mip.BulkData.Unlock();

			delete[] Pixels;
		}

		const float LengthX = GridWidth * GridPointScale;
		const float LengthY = GridHeight * GridPointScale;
		
		// Assumes the grid plane plane mesh has a size of 100x100 units at 1x1x1 scale.
		GridPlaneComponent->SetRelativeScale3D(FVector(LengthX * 0.01f, LengthY * 0.01f, 1.f));

		PlaneMaterial->SetTextureParameterValue(TEXT("GridTexture"), GridPlaneTexture);
	}
}

int AFGGrid::GetMiddleIndex() const
{
	const int HalfWidth = GridWidth / 2;
	const int HalfHeight = GridHeight / 2;
			
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

void AFGGrid::GetNeighbors(int CurrentGridIndex, TArray<int>& OutNeighbors)
{
	const int Num = GridPoints.Num();

	if(IsValidIndex(CurrentGridIndex))
	{
		UE_LOG(LogGrid, Warning, TEXT("Couldn't get neighbors since current index is invalid"));
		return;
	}
	
	OutNeighbors.Add(CheckValidIndex(CurrentGridIndex - GridWidth));
	OutNeighbors.Add(CheckValidIndex(CurrentGridIndex + GridWidth));

	int Current = CurrentGridIndex;
	// todo finish
	for(int i = 0; i < 3; i++)
	{
		OutNeighbors.Add(CurrentGridIndex + 1);
		Current = OutNeighbors[i];
	}
	

	int Remainder = CurrentGridIndex % GridWidth;
}
