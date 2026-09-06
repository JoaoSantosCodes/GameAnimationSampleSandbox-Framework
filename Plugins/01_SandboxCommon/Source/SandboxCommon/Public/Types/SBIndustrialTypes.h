#pragma once

#include "CoreMinimal.h"
#include "Types/SBPipeNetworkTypes.h"
#include "SBIndustrialTypes.generated.h"

UENUM(BlueprintType)
enum class ESBProcessorType : uint8
{
	Smelter          UMETA(DisplayName = "Smelter"),
	Foundry          UMETA(DisplayName = "Foundry"),
	Assembler        UMETA(DisplayName = "Assembler"),
	Refinery         UMETA(DisplayName = "Refinery"),
	ChemicalPlant    UMETA(DisplayName = "ChemicalPlant"),
	Constructor      UMETA(DisplayName = "Constructor")
};

UENUM(BlueprintType)
enum class ESBProcessorState : uint8
{
	Idle                  UMETA(DisplayName = "Idle"),
	Processing            UMETA(DisplayName = "Processing"),
	MissingIngredients    UMETA(DisplayName = "MissingIngredients"),
	NoPower               UMETA(DisplayName = "NoPower"),
	OutputFull            UMETA(DisplayName = "OutputFull"),
	Overheated            UMETA(DisplayName = "Overheated")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBIndustrialIngredient
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBIndustrialFluidIngredient
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	ESBFluidType FluidType = ESBFluidType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	float Volume = 0.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBIndustrialRecipe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	FName RecipeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	float CraftingTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	float PowerRequirement = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	TArray<FSBIndustrialIngredient> InputItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	TArray<FSBIndustrialFluidIngredient> InputFluids;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	TArray<FSBIndustrialIngredient> OutputItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	TArray<FSBIndustrialFluidIngredient> OutputFluids;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	float HeatGeneration = 5.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBProcessorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	ESBProcessorType ProcessorType = ESBProcessorType::Smelter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	ESBProcessorState ProcessorState = ESBProcessorState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	FSBIndustrialRecipe ActiveRecipe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	float CurrentProgressAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	float OverclockMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	float CurrentTemperature = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	float MaxSafeTemperature = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	bool bHasPower = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	int32 CompletedCyclesCount = 0;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBProcessorSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	int32 InputItemBufferCapacity = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	int32 OutputItemBufferCapacity = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	float FluidBufferCapacity = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Industrial")
	float HeatDissipationRate = 2.0f;
};
