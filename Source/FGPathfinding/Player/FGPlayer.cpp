#include "FGPlayer.h"

#include "FGPathfinding/FGGameState.h"
#include "FGPathfinding/FGGrid.h"
#include "FGPathfinding/FGPathfindingLibrary.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogFGPlayer);

AFGPlayer::AFGPlayer()
{
	
}

void AFGPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void AFGPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAction(TEXT("Select"), IE_Pressed, this, &AFGPlayer::OnSelect);
	PlayerInputComponent->BindAction(TEXT("ToggleMouseLock"), IE_Pressed, this, &AFGPlayer::OnToggleMouseLock);
}

void AFGPlayer::OnSelect()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if(PlayerController == nullptr)
	{
		UE_LOG(LogFGPlayer, Warning, TEXT("Player controller was null"));
		return;
	}

	if(!PlayerController->ShouldShowMouseCursor())
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
	
	TArray<int> Path;
	UFGPathfindingLibrary::GenerateAStarPath(Grid, IndexForStartPath, IndexForEndPath, Path);
	TArray<FColorIndexPair> ColorIndexPairs;
	
	for(int i = 0; i < Path.Num(); i++)
	{
		const int CurrentIndex = Path[i];
		if(CurrentIndex == IndexForStartPath || CurrentIndex == IndexForEndPath)
			continue;
		
		ColorIndexPairs.Add(FColorIndexPair(CurrentIndex, AIPathColor));
		Grid->SetPixelsOnPlaneTexture(ColorIndexPairs);
	}
}

void AFGPlayer::OnToggleMouseLock()
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

bool AFGPlayer::GetMouseLocationOnGrid(FVector& OutWorldMouseLocation, AFGGrid*& OutGrid) const
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
	
	OutGrid = GetClosestGrid(GameState, GetActorLocation());

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

AFGGrid* AFGPlayer::GetClosestGrid(AFGGameState* GameState, const FVector BasisLocation)
{
	const int Num = GameState->GridsInWorld.Num();

	if(Num == 0)
		return nullptr;
	
	if(Num > 1)
	{
		float ShortestSqrDistance = 1000000000.f;
		int Index = -1;
		
		for(int i = 0; i < GameState->GridsInWorld.Num(); i++)
		{
			const float Dist = FVector::DistSquared(BasisLocation, GameState->GridsInWorld[i]->GetActorLocation());

			if(Dist < ShortestSqrDistance)
			{
				ShortestSqrDistance = Dist;
				Index = i;
			}
		}
		return GameState->GridsInWorld[Index];
	}

	return GameState->GridsInWorld[0];
}
