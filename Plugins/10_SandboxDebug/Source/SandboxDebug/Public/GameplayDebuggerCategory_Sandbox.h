#pragma once

#include "CoreMinimal.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory.h"

class AActor;
class APlayerController;
class FGameplayDebuggerCanvasContext;

class FGameplayDebuggerCategory_Sandbox : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_Sandbox();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

protected:
	struct FRepData
	{
		FString ActorName;
		TArray<FString> DebugLines;

		void Serialize(FArchive& Ar)
		{
			Ar << ActorName;
			Ar << DebugLines;
		}
	};

	FRepData DataPack;
};
#endif
