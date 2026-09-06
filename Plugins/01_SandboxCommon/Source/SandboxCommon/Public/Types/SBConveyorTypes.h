#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBConveyorTypes.generated.h"

UENUM(BlueprintType)
enum class ESBConveyorNodeType : uint8
{
	BeltSegment          UMETA(DisplayName = "BeltSegment"),
	Splitter             UMETA(DisplayName = "Splitter"),
	Merger               UMETA(DisplayName = "Merger"),
	SmartSorter          UMETA(DisplayName = "SmartSorter"),
	ContainerLoader      UMETA(DisplayName = "ContainerLoader"),
	ContainerUnloader    UMETA(DisplayName = "ContainerUnloader")
};

UENUM(BlueprintType)
enum class ESBConveyorState : uint8
{
	Idle         UMETA(DisplayName = "Idle"),
	Conveying    UMETA(DisplayName = "Conveying"),
	Sorting      UMETA(DisplayName = "Sorting"),
	Merging      UMETA(DisplayName = "Merging"),
	Jammed       UMETA(DisplayName = "Jammed")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBConveyorItemSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	FGameplayTag ItemCategoryTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	float BeltProgressAlpha = 0.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBConveyorNodeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	ESBConveyorNodeType NodeType = ESBConveyorNodeType::BeltSegment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	ESBConveyorState ConveyorState = ESBConveyorState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	float BeltSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	float BeltLength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	int32 MaxItemCapacity = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	int32 CurrentItemCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	int32 SplitterRoundRobinIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	FGameplayTag FilterTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	bool bIsJammed = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBConveyorSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	float MaxConnectionDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	int32 MaxInputConnections = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	int32 MaxOutputConnections = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	float ItemTransferDelay = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	float MinimumItemSpacing = 50.0f;
};
