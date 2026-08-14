#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory_Sandbox.h"
#include "Interfaces/SBDebugInterface.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PlayerController.h"

FGameplayDebuggerCategory_Sandbox::FGameplayDebuggerCategory_Sandbox()
{
	bShowOnlyWithDebugActor = false;
	SetDataPackReplication(&DataPack);
}

void FGameplayDebuggerCategory_Sandbox::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	if (!DebugActor) return;

	DataPack.ActorName = DebugActor->GetName();
	DataPack.DebugLines.Empty();

	auto ProcessLines = [this](const TArray<FSBDebugLine>& Lines)
	{
		for (const FSBDebugLine& Line : Lines)
		{
			if (Line.bIsHeader)
			{
				DataPack.DebugLines.Add(FString::Printf(TEXT("[H]%s"), *Line.Label));
			}
			else
			{
				DataPack.DebugLines.Add(FString::Printf(TEXT("%s: %s"), *Line.Label, *Line.Value));
			}
		}
	};

	// 1. Coleta dados do próprio Actor (se implementar a interface)
	if (DebugActor->Implements<USBDebugInterface>())
	{
		TArray<FSBDebugLine> ActorLines;
		ISBDebugInterface::Execute_GetDebugDescription(DebugActor, ActorLines);
		ProcessLines(ActorLines);
	}

	// 2. Coleta dados de todos os seus componentes que implementam a interface
	TInlineComponentArray<UActorComponent*> Comps(DebugActor);
	for (UActorComponent* Comp : Comps)
	{
		if (Comp && Comp->Implements<USBDebugInterface>())
		{
			TArray<FSBDebugLine> CompLines;
			ISBDebugInterface::Execute_GetDebugDescription(Comp, CompLines);
			ProcessLines(CompLines);
		}
	}
}

void FGameplayDebuggerCategory_Sandbox::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	CanvasContext.Printf(TEXT("Inspecting Actor: {yellow}%s"), *DataPack.ActorName);
	CanvasContext.Printf(TEXT("{white}--------------------------------------------------"));

	for (const FString& Line : DataPack.DebugLines)
	{
		if (Line.StartsWith(TEXT("[H]")))
		{
			FString HeaderText = Line.Mid(3);
			CanvasContext.Printf(TEXT("\n{cyan}%s"), *HeaderText);
		}
		else
		{
			CanvasContext.Printf(TEXT("  {white}%s"), *Line);
		}
	}
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_Sandbox::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_Sandbox());
}
#endif
