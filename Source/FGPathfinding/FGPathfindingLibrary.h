#pragma once
#include "FGPathfindingLibrary.generated.h"

class AFGGrid;
UCLASS()
class UFGPathfindingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void GenerateAStarPath(AFGGrid* Grid, int StartIndex, int EndIndex, TArray<int>& OutPath);
};
