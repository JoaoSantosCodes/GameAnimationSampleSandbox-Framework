#pragma once

#include "CoreMinimal.h"
#include "Types/SBPipeNetworkTypes.h"
#include "SBExtractorTypes.generated.h"

UENUM(BlueprintType)
enum class ESBExtractorType : uint8
{
	MiningDrill          UMETA(DisplayName = "MiningDrill"),
	OilWellPump          UMETA(DisplayName = "OilWellPump"),
	GeothermalExtractor  UMETA(DisplayName = "GeothermalExtractor"),
	DeepCoreHarvester    UMETA(DisplayName = "DeepCoreHarvester"),
	WaterExtractor       UMETA(DisplayName = "WaterExtractor")
};

UENUM(BlueprintType)
enum class ESBExtractorState : uint8
{
	Idle                 UMETA(DisplayName = "Idle"),
	Extracting           UMETA(DisplayName = "Extracting"),
	Depleted             UMETA(DisplayName = "Depleted"),
	NoPower              UMETA(DisplayName = "NoPower"),
	OutputBlocked        UMETA(DisplayName = "OutputBlocked"),
	Overheated           UMETA(DisplayName = "Overheated")
};

UENUM(BlueprintType)
enum class ESBResourceDepositPurity : uint8
{
	Impure               UMETA(DisplayName = "Impure (0.5x)"),
	Normal               UMETA(DisplayName = "Normal (1.0x)"),
	Pure                 UMETA(DisplayName = "Pure (2.0x)")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBExtractorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	ESBExtractorType ExtractorType = ESBExtractorType::MiningDrill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	ESBExtractorState ExtractorState = ESBExtractorState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	ESBResourceDepositPurity DepositPurity = ESBResourceDepositPurity::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	FName ExtractedItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	ESBFluidType ExtractedFluidType = ESBFluidType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	float BaseExtractionRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	float CurrentProgressAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	float PowerConsumption = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	float OverclockMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	float CurrentTemperature = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	float MaxSafeTemperature = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	float HeatGenerationRate = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	bool bHasPower = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	bool bIsDepositDepleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	int32 TotalExtractedItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	float TotalExtractedFluids = 0.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBExtractorSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	int32 OutputItemBufferCapacity = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	float OutputFluidBufferCapacity = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extractor")
	float HeatDissipationRate = 2.0f;
};
