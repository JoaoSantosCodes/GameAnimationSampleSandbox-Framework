#pragma once

#include "CoreMinimal.h"
#include "SBWorldIntegrityTypes.generated.h"

/**
 * Nível de severidade de um problema de integridade do mundo
 */
UENUM(BlueprintType)
enum class ESBIntegritySeverity : uint8
{
	Info UMETA(DisplayName = "Info"),
	Warning UMETA(DisplayName = "Warning"),
	Error UMETA(DisplayName = "Error"),
	Critical UMETA(DisplayName = "Critical")
};

/**
 * Registro individual de inconformidade ou problema de integridade
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBIntegrityIssue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldIntegrity")
	FName IssueId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldIntegrity")
	FString SourceAssetOrEntity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldIntegrity")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldIntegrity")
	ESBIntegritySeverity Severity = ESBIntegritySeverity::Warning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldIntegrity")
	bool bAutoFixable = false;

	FSBIntegrityIssue() = default;

	FSBIntegrityIssue(FName InId, const FString& InSource, const FString& InDesc, ESBIntegritySeverity InSev = ESBIntegritySeverity::Warning, bool bInAutoFix = false)
		: IssueId(InId), SourceAssetOrEntity(InSource), Description(InDesc), Severity(InSev), bAutoFixable(bInAutoFix) {}
};

/**
 * Relatório consolidado de auditoria de integridade do mundo
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBWorldIntegrityReport
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldIntegrity")
	int32 TotalIssuesFound = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldIntegrity")
	int32 CriticalErrorsCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldIntegrity")
	int32 WarningsCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldIntegrity")
	int32 AutoFixedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldIntegrity")
	TArray<FSBIntegrityIssue> Issues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldIntegrity")
	bool bPassed = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnIntegrityReportGenerated, const FSBWorldIntegrityReport&, Report);
