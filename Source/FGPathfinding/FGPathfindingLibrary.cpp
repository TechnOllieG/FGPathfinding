#include "FGPathfindingLibrary.h"

#include "FGGrid.h"

DEFINE_LOG_CATEGORY(LogFGPathfindingLibrary);

void UFGPathfindingLibrary::GenerateAStarPath(AFGGrid* Grid, int StartIndex, int EndIndex, TArray<int>& OutPath, int MaxIterations)
{
	TArray<FAStarNode> OpenList;
	TArray<FAStarNode> ClosedList;

	FIntPoint StartCoords = Grid->ToGridCoords(StartIndex);
	FIntPoint EndCoords = Grid->ToGridCoords(EndIndex);

	OpenList.Add(FAStarNode(StartIndex, StartCoords, 0.f,
	                        Dist(StartCoords, EndCoords)));

	int Iterations = 0;
	while (OpenList.Num() > 0/* && Iterations < MaxIterations*/)
	{
		if (!OneIterationOfAStar(Grid, StartIndex, EndIndex, OpenList, ClosedList, OutPath, StartCoords, EndCoords))
			return;

		++Iterations;
	}
}

bool UFGPathfindingLibrary::OneIterationOfAStar(AFGGrid* Grid, int StartIndex, int EndIndex, TArray<FAStarNode>& OpenList, TArray<FAStarNode>& ClosedList,
	TArray<int>& OutPath, FIntPoint& StartCoords, FIntPoint& EndCoords)
{
	if (StartIndex == EndIndex || StartIndex < 0 || StartIndex >= Grid->GridPoints.Num() || EndIndex < 0 || EndIndex >=
		Grid->GridPoints.Num() || OutPath.Num() > 0)
		return false;

	if (StartCoords == INDEX_NONE || EndCoords == INDEX_NONE)
	{
		StartCoords = Grid->ToGridCoords(StartIndex);
		EndCoords = Grid->ToGridCoords(EndIndex);
	}

	if (OpenList.Num() == 0 && ClosedList.Num() == 0)
		OpenList.Add(FAStarNode(StartIndex, StartCoords, 0.f, Dist(StartCoords, EndCoords)));

	FAStarNode Current;
	OpenList.HeapPop(Current);
	FAStarNode* Parent = &ClosedList[ClosedList.Add(Current)];

	TArray<int> Neighbors;
	Grid->GetNeighbors(Current.GridIndex, Neighbors);
	CleanNeighbors(Grid, Neighbors, Current.GridIndex);

	for (int i = 0; i < Neighbors.Num(); i++)
	{
		const int CurrentNeighbor = Neighbors[i];

		const FIntPoint Coords = Grid->ToGridCoords(CurrentNeighbor);
		const float GMultiplier = Grid->GetCostForGridIndex(CurrentNeighbor);

		if(CurrentNeighbor == EndIndex)
		{
			FAStarNode Temp = FAStarNode();
			Temp.GridIndex = CurrentNeighbor;
			Temp.Previous = Parent;
				
			FAStarNode* CurrentNode = &Temp;

			while(CurrentNode->Previous != nullptr)
			{
				int Item = CurrentNode->GridIndex;
				OutPath.Insert(Item, 0);
				CurrentNode = CurrentNode->Previous;

				if(OutPath.Num() == 1)
					continue;
				
				for(int j = 1; j < OutPath.Num(); j++)
				{
					if(OutPath[j] == Item)
					{
						UE_LOG(LogFGPathfindingLibrary, Warning, TEXT("Infinitely looped path detected, exiting..."));
						return true;
					}
				}
			}
			return true;
		}

		const float GCost = Dist(Current.GridCoordinate, Coords) * GMultiplier + Current.GCost;
		const float HCost = Dist(Coords, Grid->ToGridCoords(EndIndex));
		const float Total = GCost + HCost;

		FAStarNode* Node = nullptr;
		if (ContainsIndex(OpenList, CurrentNeighbor, Node) || ContainsIndex(ClosedList, CurrentNeighbor, Node))
		{
			if (Node->Total > Total)
			{
				Node->Heuristic = HCost;
				Node->GCost = GCost;
				Node->Total = Total;
				Node->Previous = Parent;
			}

			continue;
		}

		OpenList.Add(FAStarNode(CurrentNeighbor, Coords, GCost, HCost, Parent));
	}

	OpenList.Sort();
	return true;
}

void UFGPathfindingLibrary::CleanNeighbors(AFGGrid* Grid, TArray<int>& Neighbors, int Parent)
{
	int TotalIndex = -1;
	for(int i = 0; i < Neighbors.Num(); i++)
	{
		TotalIndex++;
		const int Index = Neighbors[i];
		
		const bool CurrentIsDiagonal = TotalIndex >= 4;
		FIntPoint DirectionToCheckIn = Grid->ToGridCoords(Parent) - Grid->ToGridCoords(Index);
		
		if(Index < 0 || Grid->GetCostForGridIndex(Index) < 0 ||
			(CurrentIsDiagonal && (Grid->GetCostForGridIndex(Index + DirectionToCheckIn.X) < 0 && Grid->GetCostForGridIndex(Index + Grid->GridWidth * DirectionToCheckIn.Y) < 0)))
		{
			Neighbors.RemoveAt(i);
			--i;
		}
	}
}

int UFGPathfindingLibrary::Dist(const FIntPoint& A, const FIntPoint& B)
{
	return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
}

bool UFGPathfindingLibrary::ContainsIndex(TArray<FAStarNode>& Array, int Index, FAStarNode*& OutElement)
{
	for (int i = 0; i < Array.Num(); i++)
	{
		if (Array[i].GridIndex == Index)
		{
			OutElement = &Array[i];
			return true;
		}
	}
	return false;
}
