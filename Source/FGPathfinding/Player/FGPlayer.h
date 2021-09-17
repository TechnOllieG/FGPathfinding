#pragma once
#include "GameFramework/DefaultPawn.h"
#include "FGPlayer.generated.h"

UCLASS()
class FGPATHFINDING_API AFGPlayer : public ADefaultPawn
{
	GENERATED_BODY()

public:
	AFGPlayer();
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
