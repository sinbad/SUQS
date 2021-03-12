#include "SuqsTestModule.h"

DEFINE_LOG_CATEGORY(LogSuqsTestModule)

void FSuqsTestModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogSuqsTestModule, Log, TEXT("SUQSTest Module Started"))
}

void FSuqsTestModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogSuqsTestModule, Log, TEXT("SUQSTest Module Stopped"))
}


IMPLEMENT_MODULE(FSuqsTestModule, SUQSTest)
