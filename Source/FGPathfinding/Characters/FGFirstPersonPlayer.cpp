#include "FGFirstPersonPlayer.h"

#include "Kismet/KismetMathLibrary.h"

AFGFirstPersonPlayer::AFGFirstPersonPlayer()
{
	
}

void AFGFirstPersonPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void AFGFirstPersonPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("DefaultPawn_MoveForward", this, &AFGFirstPersonPlayer::MoveForward);
	PlayerInputComponent->BindAxis("DefaultPawn_MoveRight", this, &AFGFirstPersonPlayer::MoveRight);
	PlayerInputComponent->BindAxis("DefaultPawn_Turn", this, &AFGFirstPersonPlayer::AddControllerYawInput);
	PlayerInputComponent->BindAxis("DefaultPawn_LookUp", this, &AFGFirstPersonPlayer::AddControllerPitchInput);
	PlayerInputComponent->BindAction("Jump", IE_Pressed,  this, &AFGFirstPersonPlayer::HandlePressJump);
	PlayerInputComponent->BindAction("Jump", IE_Released,  this, &AFGFirstPersonPlayer::HandleReleaseJump);
}

void AFGFirstPersonPlayer::HandlePressJump()
{
	Jump();
}

void AFGFirstPersonPlayer::HandleReleaseJump()
{
	StopJumping();
}

void AFGFirstPersonPlayer::MoveForward(float Value)
{
	if(Controller)
	{
		FRotator const ControlSpaceRot = Controller->GetControlRotation();
		FVector Forward = UKismetMathLibrary::GetForwardVector(ControlSpaceRot);
		Forward.Z = 0;
		AddMovementInput(Forward, Value);
	}
}

void AFGFirstPersonPlayer::MoveRight(float Value)
{
	if(Controller)
	{
		FRotator const ControlSpaceRot = Controller->GetControlRotation();
		FVector Right = UKismetMathLibrary::GetRightVector(ControlSpaceRot);
		Right.Z = 0;
		AddMovementInput(Right, Value);
	}
}
