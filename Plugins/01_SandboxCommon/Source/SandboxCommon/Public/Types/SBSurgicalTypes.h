#pragma once

#include "CoreMinimal.h"
#include "SBSurgicalTypes.generated.h"

UENUM(BlueprintType)
enum class ESBSurgicalLimbType : uint8
{
	None        UMETA(DisplayName = "None"),
	LeftArm     UMETA(DisplayName = "LeftArm"),
	RightArm    UMETA(DisplayName = "RightArm"),
	LeftLeg     UMETA(DisplayName = "LeftLeg"),
	RightLeg    UMETA(DisplayName = "RightLeg")
};

UENUM(BlueprintType)
enum class ESBProstheticGrade : uint8
{
	None                UMETA(DisplayName = "None"),
	BasicProsthetic     UMETA(DisplayName = "BasicProsthetic"),
	BionicAdvanced      UMETA(DisplayName = "BionicAdvanced"),
	CyberneticAugment   UMETA(DisplayName = "CyberneticAugment")
};

UENUM(BlueprintType)
enum class ESBSurgicalOperationState : uint8
{
	Idle                UMETA(DisplayName = "Idle"),
	PreOpAnesthesia     UMETA(DisplayName = "PreOpAnesthesia"),
	InSurgery           UMETA(DisplayName = "InSurgery"),
	PostOpRecovery      UMETA(DisplayName = "PostOpRecovery")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBProstheticLimb
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surgery|Prosthetics")
	ESBSurgicalLimbType LimbType = ESBSurgicalLimbType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surgery|Prosthetics")
	ESBProstheticGrade Grade = ESBProstheticGrade::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surgery|Prosthetics")
	float Efficiency = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surgery|Prosthetics")
	float StructuralDurability = 100.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBSurgicalPatientData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surgery|Patient")
	ESBSurgicalOperationState OperationState = ESBSurgicalOperationState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surgery|Patient")
	TArray<FSBProstheticLimb> InstalledProsthetics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surgery|Patient")
	float OrganHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surgery|Patient")
	float ImmunosuppressantLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surgery|Patient")
	float OperationProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surgery|Patient")
	bool bIsAnesthetized = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surgery|Patient")
	bool bIsOrganRejectionRisk = false;
};
