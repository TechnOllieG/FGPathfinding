using UnrealBuildTool;

public class FGPathfindingEditor : ModuleRules 
{

	public FGPathfindingEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.AddRange(new string[] { "FGPathfindingEditor" });

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "FGPathfinding" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
	}
}