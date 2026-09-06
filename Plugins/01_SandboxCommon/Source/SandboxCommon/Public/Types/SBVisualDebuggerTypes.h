#pragma once

#include "CoreMinimal.h"
#include "SBVisualDebuggerTypes.generated.h"

/**
 * Categorias de visualização para depuração visual no Viewport
 */
UENUM(BlueprintType)
enum class ESBOverlayCategory : uint8
{
	PowerGrid UMETA(DisplayName = "Power Grid"),
	PipeNetwork UMETA(DisplayName = "Pipe Network"),
	ConveyorNetwork UMETA(DisplayName = "Conveyor Network"),
	DroneRoutes UMETA(DisplayName = "Drone Routes"),
	CombatHitboxes UMETA(DisplayName = "Combat Hitboxes"),
	All UMETA(DisplayName = "All Overlays")
};

/**
 * Item individual de renderização de depuração visual
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBOverlayRenderItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VisualDebugger")
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VisualDebugger")
	FVector EndLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VisualDebugger")
	FColor Color = FColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VisualDebugger")
	FString DebugText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VisualDebugger")
	float Thickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VisualDebugger")
	ESBOverlayCategory Category = ESBOverlayCategory::PowerGrid;

	FSBOverlayRenderItem() = default;

	FSBOverlayRenderItem(const FVector& InStart, const FVector& InEnd, const FColor& InColor, const FString& InText, ESBOverlayCategory InCat = ESBOverlayCategory::PowerGrid, float InThickness = 2.0f)
		: StartLocation(InStart), EndLocation(InEnd), Color(InColor), DebugText(InText), Thickness(InThickness), Category(InCat) {}
};

/**
 * Métricas do subsistema de depuração visual
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBVisualDebuggerMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VisualDebugger")
	int32 ActiveOverlaysCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VisualDebugger")
	int32 TotalRenderItemsQueued = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VisualDebugger")
	int32 EnabledCategoriesMask = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnOverlayCategoryToggled, ESBOverlayCategory, Category, bool, bEnabled);
