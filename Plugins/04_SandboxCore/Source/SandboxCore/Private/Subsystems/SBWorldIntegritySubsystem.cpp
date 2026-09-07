// Copyright 2026 João Santos. All Rights Reserved.
#include "Subsystems/SBWorldIntegritySubsystem.h"

USBWorldIntegritySubsystem::USBWorldIntegritySubsystem()
{
}

void USBWorldIntegritySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetSubsystem();
}

void USBWorldIntegritySubsystem::Deinitialize()
{
	ResetSubsystem();
	Super::Deinitialize();
}

void USBWorldIntegritySubsystem::RegisterIssue(const FSBIntegrityIssue& Issue)
{
	RegisteredIssues.Add(Issue);
}

bool USBWorldIntegritySubsystem::AuditRecipe(FName RecipeId, int32 IngredientCount, int32 OutputCount)
{
	if (IngredientCount <= 0 || OutputCount <= 0)
	{
		FSBIntegrityIssue Issue(RecipeId, TEXT("RecipeDataAsset"), FString::Printf(TEXT("Recipe %s has invalid ingredients (%d) or outputs (%d)"), *RecipeId.ToString(), IngredientCount, OutputCount), ESBIntegritySeverity::Error, false);
		RegisterIssue(Issue);
		return false;
	}
	return true;
}

bool USBWorldIntegritySubsystem::AuditLootTable(FName LootTableId, float TotalWeight)
{
	if (TotalWeight <= 0.0f)
	{
		FSBIntegrityIssue Issue(LootTableId, TEXT("LootTableDataAsset"), FString::Printf(TEXT("Loot table %s has zero or negative weight (%f)"), *LootTableId.ToString(), TotalWeight), ESBIntegritySeverity::Critical, true);
		RegisterIssue(Issue);
		return false;
	}
	return true;
}

bool USBWorldIntegritySubsystem::AuditNetworkConnection(FName NodeId, bool bHasPowerSource, bool bHasConsumer)
{
	if (!bHasPowerSource && bHasConsumer)
	{
		FSBIntegrityIssue Issue(NodeId, TEXT("PowerGrid"), FString::Printf(TEXT("Grid node %s has consumer without active power source"), *NodeId.ToString()), ESBIntegritySeverity::Warning, true);
		RegisterIssue(Issue);
		return false;
	}
	return true;
}

int32 USBWorldIntegritySubsystem::AutoFixIssues()
{
	int32 Fixed = 0;
	for (int32 i = RegisteredIssues.Num() - 1; i >= 0; --i)
	{
		if (RegisteredIssues[i].bAutoFixable)
		{
			RegisteredIssues.RemoveAt(i);
			Fixed++;
		}
	}
	LastReport.AutoFixedCount += Fixed;
	return Fixed;
}

FSBWorldIntegrityReport USBWorldIntegritySubsystem::GenerateReport()
{
	FSBWorldIntegrityReport Report;
	Report.TotalIssuesFound = RegisteredIssues.Num();
	Report.AutoFixedCount = LastReport.AutoFixedCount;
	Report.Issues = RegisteredIssues;

	for (const FSBIntegrityIssue& Issue : RegisteredIssues)
	{
		if (Issue.Severity == ESBIntegritySeverity::Critical || Issue.Severity == ESBIntegritySeverity::Error)
		{
			Report.CriticalErrorsCount++;
		}
		else if (Issue.Severity == ESBIntegritySeverity::Warning)
		{
			Report.WarningsCount++;
		}
	}

	Report.bPassed = (Report.CriticalErrorsCount == 0);
	LastReport = Report;
	OnIntegrityReportGenerated.Broadcast(Report);
	return Report;
}

void USBWorldIntegritySubsystem::ResetSubsystem()
{
	RegisteredIssues.Empty();
	LastReport = FSBWorldIntegrityReport();
}
