#pragma once
#include "GameFramework/GameState.h"
#include "FGGameState.generated.h"

class AFGGrid;
UCLASS()
class AFGGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AFGGrid*> GridsInWorld;

	AFGGrid* GetClosestGrid(const FVector BasisLocation);
};
