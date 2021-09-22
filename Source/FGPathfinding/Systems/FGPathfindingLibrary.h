#pragma once
#include "FGPathfindingLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFGPathfindingLibrary, Log, All);

struct FAStarNode
{	
	FAStarNode() {}	
	FAStarNode(int GridIndex, FIntPoint GridCoordinate, float GCost, float Heuristic, FAStarNode* Previous = nullptr)
	{
		this->GridIndex = GridIndex;
		this->GridCoordinate = GridCoordinate;
		this->GCost = GCost;
		this->Heuristic = Heuristic;
		this->Total = GCost + Heuristic;
		this->Previous = Previous;
	}

	FAStarNode* Previous;
	int GridIndex = -1;
	FIntPoint GridCoordinate;
	float GCost; // The distance from the start node
	float Heuristic; // The distance to the end node
	float Total; // Total of both

	bool operator<(const FAStarNode& B) const
	{		
		return Total < B.Total;
	}
};

class AFGGrid;
UCLASS()
class UFGPathfindingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void GenerateAStarPath(AFGGrid* Grid, int StartIndex, int EndIndex, TArray<int>& OutPath, int MaxIterations = -1);

	/** Returns true on non error */
	static bool OneIterationOfAStar(AFGGrid* Grid, int StartIndex, int EndIndex, TArray<FAStarNode>& OpenList, TArray<FAStarNode>& ClosedList, TArray<int>& OutPath, FIntPoint& StartCoords, FIntPoint& EndCoords);

	/** Calculates the manhattan distance between point a and b */
	static float Dist(const FIntPoint& A, const FIntPoint& B);

private:
	static bool ContainsIndex(TArray<FAStarNode>& Array, int Index, FAStarNode*& OutElement);
	/** Will remove any -1, neighbors that are obstacles and diagonals that are invalid */
	static void CleanNeighbors(AFGGrid* Grid, TArray<int>& Neighbors, int Parent);
};
