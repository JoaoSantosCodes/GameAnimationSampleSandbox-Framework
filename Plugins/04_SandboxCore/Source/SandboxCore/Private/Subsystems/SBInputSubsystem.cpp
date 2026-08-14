#include "Subsystems/SBInputSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Utilities/SBLogCategories.h"

void USBInputSubsystem::AddMappingContext(const UInputMappingContext* MappingContext, int32 Priority)
{
	if (!MappingContext) return;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	if (UEnhancedInputLocalPlayerSubsystem* EISubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		FModifyContextOptions Options;
		Options.bIgnoreAllPressedKeysUntilRelease = false;
		EISubsystem->AddMappingContext(MappingContext, Priority, Options);
		
		UE_LOG(LogSandboxCore, Log, TEXT("Added Input Mapping Context: %s (Priority: %d)"), *MappingContext->GetName(), Priority);
	}
}

void USBInputSubsystem::RemoveMappingContext(const UInputMappingContext* MappingContext)
{
	if (!MappingContext) return;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	if (UEnhancedInputLocalPlayerSubsystem* EISubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		EISubsystem->RemoveMappingContext(MappingContext);
		
		UE_LOG(LogSandboxCore, Log, TEXT("Removed Input Mapping Context: %s"), *MappingContext->GetName());
	}
}
