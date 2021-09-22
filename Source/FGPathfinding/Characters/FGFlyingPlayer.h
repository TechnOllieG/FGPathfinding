#pragma once
#include "GameFramework/DefaultPawn.h"
#include "FGPathfinding/Systems/FGPathfindingLibrary.h"
#include "FGFlyingPlayer.generated.h"

class AFGGrid;
class AFGGameState;
DECLARE_LOG_CATEGORY_EXTERN(LogFGPlayer, Log, All);

UCLASS()
class FGPATHFINDING_API AFGFlyingPlayer : public ADefaultPawn
{
	GENERATED_BODY()

public:
	AFGFlyingPlayer();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool DebugDrawGridIndices = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool StartWithMouseLocked = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool ShowDebugSphereOnClick = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool SlowAStarWithVisualization = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DelayBetweenIteration = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FColor AIPathStartColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FColor AIPathColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FColor AIPathEndColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FColor AIPathOpenColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FColor AIPathClosedColor;
	
	UFUNCTION(BlueprintCallable)
	bool GetMouseLocationOnGrid(FVector& OutWorldMouseLocation, AFGGrid*& OutGrid) const;

private:
	void OnSelect();
	void OnToggleMouseLock();
	
	int IndexForStartPath = -1;
	int IndexForEndPath = -1;
	bool RunAStarIteratively = false;

	UPROPERTY()
	AFGGrid* CurrentGrid;
	
	TArray<FAStarNode> CachedOpenList;
	TArray<FAStarNode> CachedClosedList;
	FIntPoint StartCoords;
	FIntPoint EndCoords;
	float LastIterationTime = -100.f;

	void DrawPath(AFGGrid* Grid, TArray<int>& Path, FColor Color);
};
