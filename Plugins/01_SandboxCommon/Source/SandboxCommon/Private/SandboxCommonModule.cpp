#include "SandboxCommonModule.h"
#include "SBGameplayTags.h"

#define LOCTEXT_NAMESPACE "FSandboxCommonModule"

void FSandboxCommonModule::StartupModule()
{
	FSBGameplayTags::InitializeNativeTags();
}

void FSandboxCommonModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSandboxCommonModule, SandboxCommon)
