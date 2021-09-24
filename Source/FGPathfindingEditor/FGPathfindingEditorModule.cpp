#include "FGPathfindingEditorModule.h"

DEFINE_LOG_CATEGORY(LogPathfindingEditorModule);

void FFGPathfindingEditorModule::StartupModule()
{
	UE_LOG(LogPathfindingEditorModule, Log, TEXT("PathfindingEditor module startup"));
}

void FFGPathfindingEditorModule::ShutdownModule()
{
	
}
