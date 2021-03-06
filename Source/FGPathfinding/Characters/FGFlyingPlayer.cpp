#include "FGFlyingPlayer.h"

#include "FGPathfinding/FGGameState.h"
#include "FGPathfinding/Systems/FGPathfindingLibrary.h"
#include "FGPathfinding/Systems/FGGrid.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogFGPlayer);

AFGFlyingPlayer::AFGFlyingPlayer()
{
	
}

void AFGFlyingPlayer::BeginPlay()
{
	Super::BeginPlay();

	if(StartWithMouseLocked)
		OnToggleMouseLock();
}

void AFGFlyingPlayer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(CurrentGrid == nullptr)
	{
		FVector WorldMouseLocation;
		GetMouseLocationOnGrid(WorldMouseLocation, CurrentGrid);
	}

	if(DebugDrawGridIndices)
		CurrentGrid->DebugDrawGridIndices();

	const float Time = GetGameTimeSinceCreation();
	
	if(!RunAStarIteratively || Time - LastIterationTime < DelayBetweenIteration)
		return;

	TArray<int> Path;
	LastIterationTime = Time;
			
	UFGPathfindingLibrary::OneIterationOfAStar(CurrentGrid, IndexForStartPath, IndexForEndPath, CachedOpenList, CachedClosedList, Path, StartCoords, EndCoords);

	if(Path.Num() > 0)
	{
		CurrentGrid->ResetPlaneTexture();
		DrawPath(CurrentGrid, Path, AIPathColor, true);
		CachedOpenList.Empty();
		CachedClosedList.Empty();
		RunAStarIteratively = false;
		return;
	}

	TArray<FAStarNode> ClosedListValues;
	CachedClosedList.GenerateValueArray(ClosedListValues);
	
	TArray<FAStarNode>* CurrentArray = &CachedOpenList;
	FColor CurrentColor = AIPathOpenColor;
			
	for(int i = 0; i < 2; i++)
	{
		TArray<int> CurrentPath;
		for(int j = 0; j < CurrentArray->Num(); j++)
		{
			CurrentPath.Add(CurrentArray->GetData()[j].GridIndex);
		}
		DrawPath(CurrentGrid, CurrentPath, CurrentColor);
		CurrentArray = &ClosedListValues;
		CurrentColor = AIPathClosedColor;
	}
}

void AFGFlyingPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAction(TEXT("Select"), IE_Pressed, this, &AFGFlyingPlayer::OnSelect);
	PlayerInputComponent->BindAction(TEXT("ToggleMouseLock"), IE_Pressed, this, &AFGFlyingPlayer::OnToggleMouseLock);
}

void AFGFlyingPlayer::OnSelect()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if(PlayerController == nullptr)
	{
		UE_LOG(LogFGPlayer, Warning, TEXT("Player controller was null"));
		return;
	}

	if(!PlayerController->ShouldShowMouseCursor() || RunAStarIteratively)
		return;
	
	AFGGrid* Grid;
	FVector WorldMouseLocation;
	
	if(!GetMouseLocationOnGrid(WorldMouseLocation, Grid))
		return;

	const int SelectedGridIndex = Grid->GetClosestIndex(WorldMouseLocation);

	if(ShowDebugSphereOnClick)
		UKismetSystemLibrary::DrawDebugSphere(this, Grid->GridPoints[SelectedGridIndex], 25.f, 12, FLinearColor::White, 1.f);
	
	if(IndexForStartPath < 0 || IndexForEndPath >= 0)
	{
		Grid->ResetPlaneTexture();
		IndexForEndPath = -1;
		IndexForStartPath = SelectedGridIndex;
		Grid->SetPixelOnPlaneTexture(IndexForStartPath, AIPathStartColor);
		return;
	}
	
	IndexForEndPath = SelectedGridIndex;
	Grid->SetPixelOnPlaneTexture(IndexForEndPath, AIPathEndColor);
	
	if(!SlowAStarWithVisualization)
	{
		TArray<int> Path;
		UFGPathfindingLibrary::GenerateAStarPath(Grid, IndexForStartPath, IndexForEndPath, Path);
		DrawPath(Grid, Path, AIPathColor);
	}
	else
	{
		StartCoords = Grid->ToGridCoords(IndexForStartPath);
		EndCoords = Grid->ToGridCoords(IndexForEndPath);
		RunAStarIteratively = true;
		CachedOpenList.Add(FAStarNode(IndexForStartPath, StartCoords, 0.f, UFGPathfindingLibrary::Dist(StartCoords, EndCoords)));
		CurrentGrid = Grid;
	}
}

void AFGFlyingPlayer::DrawPath(AFGGrid* Grid, TArray<int>& Path, FColor Color, bool DrawStartAndEnd)
{
	TArray<FColorIndexPair> ColorIndexPairs;
	for(int i = 0; i < Path.Num(); i++)
	{
		const int CurrentIndex = Path[i];
		
		if(CurrentIndex == IndexForStartPath)
		{
			if(!DrawStartAndEnd) continue;
			ColorIndexPairs.Add(FColorIndexPair(CurrentIndex, AIPathStartColor));
			continue;
		}
		if(CurrentIndex == IndexForEndPath)
		{
			if(!DrawStartAndEnd) continue;
			ColorIndexPairs.Add(FColorIndexPair(CurrentIndex, AIPathEndColor));
			continue;
		}
		
		ColorIndexPairs.Add(FColorIndexPair(CurrentIndex, Color));
	}
	Grid->SetPixelsOnPlaneTexture(ColorIndexPairs);
}

void AFGFlyingPlayer::OnToggleMouseLock()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if(PlayerController == nullptr)
	{
		UE_LOG(LogFGPlayer, Warning, TEXT("Player controller was null"));
		return;
	}

	const bool ShowMouseCursor = PlayerController->ShouldShowMouseCursor();
	
	PlayerController->ClientIgnoreLookInput(!ShowMouseCursor);
	PlayerController->SetShowMouseCursor(!ShowMouseCursor);
}

bool AFGFlyingPlayer::GetMouseLocationOnGrid(FVector& OutWorldMouseLocation, AFGGrid*& OutGrid) const
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	AFGGameState* GameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	if (PlayerController == nullptr)
	{
		UE_LOG(LogFGPlayer, Warning, TEXT("Couldn't get mouse location on grid because player controller was null"));
		return false;
	}

	if(GameState == nullptr)
	{
		UE_LOG(LogFGPlayer, Warning, TEXT("Couldn't get mouse location on grid because game state was null"));
		return false;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;

	PlayerController->GetMousePosition(MouseX, MouseY);

	FVector MouseWorldLocation;
	FVector MouseDirection;
	
	PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, MouseWorldLocation, MouseDirection);

	if (MouseDirection.IsNearlyZero())
	{
		UE_LOG(LogFGPlayer, Warning, TEXT("Mouse Direction is not valid"));
		return false;
	}
	
	OutGrid = GameState->GetClosestGrid(GetActorLocation());

	if(OutGrid == nullptr)
	{
		UE_LOG(LogFGPlayer, Warning, TEXT("Couldn't get closest grid"));
		return false;
	}

	const FVector PlaneUp = OutGrid->GetActorUpVector();
	const FVector PlaneLocation = OutGrid->GetActorLocation();

	float D = FVector::DotProduct(-PlaneUp, PlaneLocation);
	float T = -(D + FVector::DotProduct(PlaneUp, MouseWorldLocation)) / FVector::DotProduct(PlaneUp, MouseDirection);
	OutWorldMouseLocation = MouseWorldLocation + MouseDirection * T;

	return true;
}
