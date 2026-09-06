#include "Subsystems/SBFaultToleranceSubsystem.h"

USBFaultToleranceSubsystem::USBFaultToleranceSubsystem()
{
}

void USBFaultToleranceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetSubsystem();
}

void USBFaultToleranceSubsystem::Deinitialize()
{
	ResetSubsystem();
	Super::Deinitialize();
}

void USBFaultToleranceSubsystem::RegisterService(FName ServiceName, FGameplayTag ServiceTag, float DefaultFallbackValue)
{
	FSBFallbackServiceRecord Record(ServiceName, ServiceTag, DefaultFallbackValue);
	MonitoredServices.Add(ServiceName, Record);
}

void USBFaultToleranceSubsystem::ReportServiceFailure(FName ServiceName)
{
	if (FSBFallbackServiceRecord* Rec = MonitoredServices.Find(ServiceName))
	{
		Rec->FailureCount++;
		TotalFaultsIntercepted++;

		if (Rec->FailureCount >= FallbackThreshold)
		{
			Rec->CurrentMode = ESBFaultToleranceMode::Fallback;
			Rec->bIsHealthy = false;
		}
		else if (Rec->FailureCount >= DegradedThreshold)
		{
			Rec->CurrentMode = ESBFaultToleranceMode::Degraded;
			Rec->bIsHealthy = false;
		}

		OnServiceStateChanged.Broadcast(ServiceName, Rec->CurrentMode, Rec->bIsHealthy);
	}
}

void USBFaultToleranceSubsystem::ReportServiceRecovered(FName ServiceName)
{
	if (FSBFallbackServiceRecord* Rec = MonitoredServices.Find(ServiceName))
	{
		Rec->FailureCount = 0;
		Rec->CurrentMode = ESBFaultToleranceMode::Normal;
		Rec->bIsHealthy = true;
		TotalRecoveries++;

		OnServiceStateChanged.Broadcast(ServiceName, Rec->CurrentMode, Rec->bIsHealthy);
	}
}

void USBFaultToleranceSubsystem::SetServiceMode(FName ServiceName, ESBFaultToleranceMode NewMode)
{
	if (FSBFallbackServiceRecord* Rec = MonitoredServices.Find(ServiceName))
	{
		Rec->CurrentMode = NewMode;
		Rec->bIsHealthy = (NewMode == ESBFaultToleranceMode::Normal);
		OnServiceStateChanged.Broadcast(ServiceName, Rec->CurrentMode, Rec->bIsHealthy);
	}
}

ESBFaultToleranceMode USBFaultToleranceSubsystem::GetServiceMode(FName ServiceName) const
{
	if (const FSBFallbackServiceRecord* Rec = MonitoredServices.Find(ServiceName))
	{
		return Rec->CurrentMode;
	}
	return ESBFaultToleranceMode::Disabled;
}

bool USBFaultToleranceSubsystem::IsServiceHealthy(FName ServiceName) const
{
	if (const FSBFallbackServiceRecord* Rec = MonitoredServices.Find(ServiceName))
	{
		return Rec->bIsHealthy;
	}
	return false;
}

float USBFaultToleranceSubsystem::QuerySafeValue(FName ServiceName, float RawValue, bool bServiceValid) const
{
	const FSBFallbackServiceRecord* Rec = MonitoredServices.Find(ServiceName);
	if (!Rec)
	{
		return bServiceValid ? RawValue : 0.0f;
	}

	if (!bServiceValid || Rec->CurrentMode == ESBFaultToleranceMode::Fallback || Rec->CurrentMode == ESBFaultToleranceMode::Disabled)
	{
		return Rec->FallbackDefaultValue;
	}

	if (Rec->CurrentMode == ESBFaultToleranceMode::Degraded)
	{
		return (RawValue + Rec->FallbackDefaultValue) * 0.5f;
	}

	return RawValue;
}

FSBFaultToleranceMetrics USBFaultToleranceSubsystem::GetMetrics() const
{
	FSBFaultToleranceMetrics Metrics;
	Metrics.TotalRegisteredServices = MonitoredServices.Num();
	Metrics.TotalFaultsIntercepted = TotalFaultsIntercepted;
	Metrics.TotalRecoveries = TotalRecoveries;

	int32 Degraded = 0;
	int32 Fallbacks = 0;
	for (const auto& Pair : MonitoredServices)
	{
		if (Pair.Value.CurrentMode == ESBFaultToleranceMode::Degraded)
		{
			Degraded++;
		}
		else if (Pair.Value.CurrentMode == ESBFaultToleranceMode::Fallback)
		{
			Fallbacks++;
		}
	}
	Metrics.ActiveDegradedServices = Degraded;
	Metrics.ActiveFallbackServices = Fallbacks;

	return Metrics;
}

void USBFaultToleranceSubsystem::ResetSubsystem()
{
	MonitoredServices.Empty();
	TotalFaultsIntercepted = 0;
	TotalRecoveries = 0;
}
