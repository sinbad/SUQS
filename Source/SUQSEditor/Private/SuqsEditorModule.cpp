#include "SuqsEditorModule.h"

#define LOCTEXT_NAMESPACE "FSuqsEditor"

DEFINE_LOG_CATEGORY(LogSuqsEditorModule)

void FSuqsEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogSuqsEditorModule, Log, TEXT("SUQSEditor Module Started"))
}

void FSuqsEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogSuqsEditorModule, Log, TEXT("SUQSEditor Module Stopped"))
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuqsEditorModule, SUQSEditor)
