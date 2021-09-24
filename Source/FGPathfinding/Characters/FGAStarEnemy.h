#pragma once
#include "GameFramework/Pawn.h"
#include "FGAStarEnemy.generated.h"

class AFGGrid;
class AFGFirstPersonPlayer;

UCLASS()
class FGPATHFINDING_API AFGAStarEnemy : public APawn
{
	GENERATED_BODY()

public:
	AFGAStarEnemy();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool DebugPath = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpeedMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ClosenessRequiredToSwitchTarget = 1.f;

private:
	UPROPERTY()
	AFGFirstPersonPlayer* PlayerToFollow;

	UPROPERTY()
	AFGGrid* Grid;

	TArray<int> CurrentPath = TArray<int>();
	int OldPlayerIndex = -1;
	int CurrentTargetIndex = 1;
	FVector TargetPos;
	bool CurrentlyMoving = false;

	void UpdateTargetPos(int NewTargetIndex);
};