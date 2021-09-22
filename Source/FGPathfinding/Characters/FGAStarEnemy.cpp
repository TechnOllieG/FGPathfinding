#include "FGAStarEnemy.h"

#include "FGFirstPersonPlayer.h"
#include "FGPathfinding/FGGameState.h"
#include "FGPathfinding/Systems/FGGrid.h"
#include "FGPathfinding/Systems/FGPathfindingLibrary.h"
#include "Kismet/GameplayStatics.h"

AFGAStarEnemy::AFGAStarEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFGAStarEnemy::BeginPlay()
{
	Super::BeginPlay();

	PlayerToFollow = Cast<AFGFirstPersonPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	Grid = Cast<AFGGameState>(UGameplayStatics::GetGameState(this))->GetClosestGrid(GetActorLocation());
}

void AFGAStarEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(PlayerToFollow == nullptr || Grid == nullptr)
		return;

	if(DebugPath)
	{
		for(int i = 0; i < CurrentPath.Num(); i++)
		{
			FLinearColor Color = FLinearColor::White;
			
			int Index = CurrentPath[i];
			
			if(i == CurrentTargetIndex)
				Color = FLinearColor::Green;

			if(i == CurrentTargetIndex - 1)
				Color = FLinearColor::Red;
				
			UKismetSystemLibrary::DrawDebugSphere(this, Grid->GridPoints[Index], 25.f, 12, Color);
		}
	}
	
	int EndIndex = Grid->GetClosestIndex(PlayerToFollow->GetActorLocation());

	if(EndIndex != OldPlayerIndex)
	{
		OldPlayerIndex = EndIndex;
		int StartIndex = Grid->GetClosestIndex(GetActorLocation());
		CurrentPath.Empty();
		
		UFGPathfindingLibrary::GenerateAStarPath(Grid, StartIndex, EndIndex, CurrentPath, 10000);

		UpdateTargetPos(1);
	}

	if(CurrentlyMoving)
	{
		AddActorWorldOffset((TargetPos - OriginPos) / CurrentDelta * DeltaSeconds * SpeedMultiplier);
		
		if((TargetPos - GetActorLocation()).SizeSquared2D() < FMath::Square(ClosenessRequiredToSwitchTarget))
		{
			UpdateTargetPos(++CurrentTargetIndex);
		}
	}
}

void AFGAStarEnemy::UpdateTargetPos(int NewTargetIndex)
{
	if(NewTargetIndex >= CurrentPath.Num())
	{
		CurrentlyMoving = false;
		return;
	}
	
	CurrentTargetIndex = NewTargetIndex;
	OriginPos = Grid->GridPoints[CurrentPath[CurrentTargetIndex - 1]];
	TargetPos = Grid->GridPoints[CurrentPath[CurrentTargetIndex]];
	CurrentDelta = (TargetPos - OriginPos).Size();
	
	if(FMath::IsNearlyZero(CurrentDelta))
		UpdateTargetPos(++NewTargetIndex);
	
	CurrentlyMoving = true;
}

