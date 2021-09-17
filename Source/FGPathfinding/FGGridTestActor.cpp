#include "FGGridTestActor.h"

#include "FGGrid.h"

AFGGridTestActor::AFGGridTestActor()
{
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	RootComponent = StaticMesh;
}

void AFGGridTestActor::BeginPlay()
{
	Super::BeginPlay();
}

void AFGGridTestActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CurrentIndex = Grid->GetClosestIndex(GetActorLocation());
	TArray<int> TempNeighbors;
	Grid->GetNeighbors(CurrentIndex, TempNeighbors);
	Neighbors = TempNeighbors;
}

