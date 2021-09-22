#pragma once
#include "GameFramework/Character.h"
#include "FGFirstPersonPlayer.generated.h"

UCLASS()
class FGPATHFINDING_API AFGFirstPersonPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	AFGFirstPersonPlayer();
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void HandlePressJump();
	void HandleReleaseJump();
};
