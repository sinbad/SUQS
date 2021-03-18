#include "SuqsModule.h"
#include "Suqs.h"

#define LOCTEXT_NAMESPACE "FSuqs"

void FSuqsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogSUQS, Log, TEXT("SUQS Module Started"))
}

void FSuqsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogSUQS, Log, TEXT("SUQS Module Stopped"))
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuqsModule, SUQS)
