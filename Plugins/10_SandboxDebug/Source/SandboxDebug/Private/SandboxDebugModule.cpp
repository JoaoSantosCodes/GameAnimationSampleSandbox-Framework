#include "SandboxDebugModule.h"
#include "Modules/ModuleManager.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "GameplayDebuggerCategory_Sandbox.h"
#endif

#define LOCTEXT_NAMESPACE "FSandboxDebugModule"

void FSandboxDebugModule::StartupModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger::Get().RegisterCategory(
			TEXT("Sandbox"),
			IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_Sandbox::MakeInstance),
			EGameplayDebuggerCategoryState::EnabledInGameAndSimulate,
			5
		);
		IGameplayDebugger::Get().NotifyCategoriesChanged();
	}
#endif
}

void FSandboxDebugModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger::Get().UnregisterCategory(TEXT("Sandbox"));
		IGameplayDebugger::Get().NotifyCategoriesChanged();
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSandboxDebugModule, SandboxDebug)
