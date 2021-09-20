#pragma once
#include "GameFramework/DefaultPawn.h"
#include "FGPlayer.generated.h"

class AFGGrid;
class AFGGameState;
DECLARE_LOG_CATEGORY_EXTERN(LogFGPlayer, Log, All);

UCLASS()
class FGPATHFINDING_API AFGPlayer : public ADefaultPawn
{
	GENERATED_BODY()

public:
	AFGPlayer();
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool ShowDebugSphereOnClick = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FColor AIPathStartColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FColor AIPathColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FColor AIPathEndColor;
	
	UFUNCTION(BlueprintCallable)
	bool GetMouseLocationOnGrid(FVector& OutWorldMouseLocation, AFGGrid*& OutGrid) const;

private:
	static AFGGrid* GetClosestGrid(AFGGameState* GameState, const FVector BasisLocation);
	void OnSelect();
	void OnToggleMouseLock();
	
	int IndexForStartPath = -1;
	int IndexForEndPath = -1;
};
