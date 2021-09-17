#pragma once
#include "GameFramework/Actor.h"
#include "FGGridTestActor.generated.h"

class AFGGrid;
UCLASS()
class AFGGridTestActor : public AActor
{
	GENERATED_BODY()

public:
	AFGGridTestActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AFGGrid* Grid;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int CurrentIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<int> Neighbors;
};