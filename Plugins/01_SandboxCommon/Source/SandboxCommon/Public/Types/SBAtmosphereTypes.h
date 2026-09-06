#pragma once

#include "CoreMinimal.h"
#include "SBAtmosphereTypes.generated.h"

UENUM(BlueprintType)
enum class ESBAtmosphericHazardType : uint8
{
	None              UMETA(DisplayName = "None"),
	Hypoxia           UMETA(DisplayName = "Hypoxia"),
	Hypercapnia       UMETA(DisplayName = "Hypercapnia"),
	ToxicGas          UMETA(DisplayName = "ToxicGas"),
	Decompression     UMETA(DisplayName = "Decompression"),
	ExtremePressure   UMETA(DisplayName = "ExtremePressure")
};

UENUM(BlueprintType)
enum class ESBSuitPressurizationState : uint8
{
	Unsealed          UMETA(DisplayName = "Unsealed"),
	Pressurized       UMETA(DisplayName = "Pressurized"),
	Compromised       UMETA(DisplayName = "Compromised"),
	Breached          UMETA(DisplayName = "Breached")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBAtmosphereEnvironmentData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Environment")
	float OxygenPercentage = 21.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Environment")
	float ToxicGasPPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Environment")
	float BarometricPressureKPa = 101.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Environment")
	bool bIsVacuum = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBAtmosphericSafetyData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Safety")
	float BloodOxygenSaturation = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Safety")
	float ToxicityLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Safety")
	float SuitOxygenReserve = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Safety")
	float MaxSuitOxygenReserve = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Safety")
	float FilterIntegrity = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Safety")
	float SuitSealIntegrity = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Safety")
	ESBSuitPressurizationState SuitState = ESBSuitPressurizationState::Unsealed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Safety")
	bool bIsHypoxic = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere|Safety")
	bool bInToxicInhalation = false;
};
