#include "FGPlayer.h"

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
	
}

bool AFGPlayer::GetMouseLocationOnGrid(FVector& OutWorldMouseLocation) const
{
	if (CurrentGridActor == nullptr)
		return false;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController == nullptr)
		return false;

	float MouseX = 0.0f;
	float MouseY = 0.0f;

	PlayerController->GetMousePosition(MouseX, MouseY);

	FVector MouseWorldLocation;
	FVector MouseDirection;
	const FVector PlaneUp = CurrentGridActor->GetActorUpVector();
	const FVector PlaneLocation = CurrentGridActor->GetActorLocation();

	PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, MouseWorldLocation, MouseDirection);

	if (MouseDirection.IsNearlyZero())
		return false;

	float D = FVector::DotProduct(-PlaneUp, PlaneLocation);
	float T = -(D + FVector::DotProduct(PlaneUp, MouseWorldLocation)) / FVector::DotProduct(PlaneUp, MouseDirection);
	OutWorldMouseLocation = MouseWorldLocation + MouseDirection * T;

	return true;
}