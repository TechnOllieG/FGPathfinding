#include "FGGameState.h"

#include "Systems/FGGrid.h"

AFGGrid* AFGGameState::GetClosestGrid(const FVector BasisLocation)
{
	const int Num = GridsInWorld.Num();

	if(Num == 0)
		return nullptr;
	
	if(Num > 1)
	{
		float ShortestSqrDistance = 1000000000.f;
		int Index = -1;
		
		for(int i = 0; i < GridsInWorld.Num(); i++)
		{
			const float Dist = FVector::DistSquared(BasisLocation, GridsInWorld[i]->GetActorLocation());

			if(Dist < ShortestSqrDistance)
			{
				ShortestSqrDistance = Dist;
				Index = i;
			}
		}
		return GridsInWorld[Index];
	}

	return GridsInWorld[0];
}
