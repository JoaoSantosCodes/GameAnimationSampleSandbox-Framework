#include "Components/SBRadiationExposureComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBRadiationExposureComponent::USBRadiationExposureComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetupRadiationComponent(0.0f, 0.0f);
}

void USBRadiationExposureComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBRadiationExposureComponent::SetupRadiationComponent(float InitialDose_mSv, float InitialShielding)
{
	ExposureData.AccumulatedDose_mSv = InitialDose_mSv;
	ExposureData.CurrentDoseRate_mSv_h = 0.0f;
	ExposureData.LeadShieldingFactor = FMath::Clamp(InitialShielding, 0.0f, 1.0f);
	ExposureData.GeigerClickFrequencyHz = 0.0f;
	ExposureData.SicknessStage = ESBRadiationSicknessStage::None;
	ExposureData.bIsGeigerClicking = false;
	SyncTags();
}

void USBRadiationExposureComponent::EquipLeadShielding(float ShieldingFactor)
{
	ExposureData.LeadShieldingFactor = FMath::Clamp(ShieldingFactor, 0.0f, 1.0f);
	SyncTags();
}

void USBRadiationExposureComponent::AdministerAntiradMedication(float DoseReductionAmount_mSv)
{
	ExposureData.AccumulatedDose_mSv = FMath::Max(0.0f, ExposureData.AccumulatedDose_mSv - DoseReductionAmount_mSv);

	ESBRadiationSicknessStage OldStage = ExposureData.SicknessStage;
	if (ExposureData.AccumulatedDose_mSv < 500.0f)
	{
		ExposureData.SicknessStage = ESBRadiationSicknessStage::None;
	}
	else if (ExposureData.AccumulatedDose_mSv < 1500.0f)
	{
		ExposureData.SicknessStage = ESBRadiationSicknessStage::MildExposure;
	}
	else if (ExposureData.AccumulatedDose_mSv < 4000.0f)
	{
		ExposureData.SicknessStage = ESBRadiationSicknessStage::AcuteRadiationSickness;
	}

	if (OldStage != ExposureData.SicknessStage)
	{
		OnRadiationStageChanged.Broadcast(OldStage, ExposureData.SicknessStage);
	}

	SyncTags();
}

void USBRadiationExposureComponent::PerformDecontamination(float SurfaceWashEfficiency)
{
	float Cleared = ExposureData.AccumulatedDose_mSv * FMath::Clamp(SurfaceWashEfficiency, 0.0f, 1.0f);
	ExposureData.AccumulatedDose_mSv = FMath::Max(0.0f, ExposureData.AccumulatedDose_mSv - Cleared);

	ESBRadiationSicknessStage OldStage = ExposureData.SicknessStage;
	if (ExposureData.AccumulatedDose_mSv < 500.0f)
	{
		ExposureData.SicknessStage = ESBRadiationSicknessStage::None;
	}

	if (OldStage != ExposureData.SicknessStage)
	{
		OnRadiationStageChanged.Broadcast(OldStage, ExposureData.SicknessStage);
	}

	OnDecontaminationCompleted.Broadcast(Cleared);
	SyncTags();
}

void USBRadiationExposureComponent::SimulateRadiationTick(float DeltaTime, const FSBRadiationEnvironmentData& Environment)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	float ShieldingMultiplier = (1.0f - ExposureData.LeadShieldingFactor);
	ExposureData.CurrentDoseRate_mSv_h = Environment.AmbientDoseRate_mSv_h * ShieldingMultiplier;

	if (Environment.AmbientDoseRate_mSv_h > 0.5f)
	{
		ExposureData.GeigerClickFrequencyHz = FMath::Clamp(Environment.AmbientDoseRate_mSv_h * 0.05f, 1.0f, 100.0f);
		ExposureData.bIsGeigerClicking = true;
		OnGeigerClick.Broadcast(ExposureData.GeigerClickFrequencyHz);
	}
	else
	{
		ExposureData.GeigerClickFrequencyHz = 0.0f;
		ExposureData.bIsGeigerClicking = false;
	}

	float DoseRatePerSecond = ExposureData.CurrentDoseRate_mSv_h / 3600.0f;
	ExposureData.AccumulatedDose_mSv += (DoseRatePerSecond * DeltaTime);

	ESBRadiationSicknessStage OldStage = ExposureData.SicknessStage;
	if (ExposureData.AccumulatedDose_mSv >= 4000.0f)
	{
		ExposureData.SicknessStage = ESBRadiationSicknessStage::CriticalLethalARS;
	}
	else if (ExposureData.AccumulatedDose_mSv >= 1500.0f)
	{
		ExposureData.SicknessStage = ESBRadiationSicknessStage::AcuteRadiationSickness;
		if (OldStage != ESBRadiationSicknessStage::AcuteRadiationSickness && OldStage != ESBRadiationSicknessStage::CriticalLethalARS)
		{
			OnAcuteRadiationSicknessTriggered.Broadcast();
		}
	}
	else if (ExposureData.AccumulatedDose_mSv >= 500.0f)
	{
		ExposureData.SicknessStage = ESBRadiationSicknessStage::MildExposure;
	}
	else
	{
		ExposureData.SicknessStage = ESBRadiationSicknessStage::None;
	}

	if (OldStage != ExposureData.SicknessStage)
	{
		OnRadiationStageChanged.Broadcast(OldStage, ExposureData.SicknessStage);
	}

	SyncTags();
}

void USBRadiationExposureComponent::SyncTags()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (!CachedStateComp.IsValid())
	{
		return;
	}

	CachedStateComp->RemoveTag(Tags.State_Radiation_Exposed);
	CachedStateComp->RemoveTag(Tags.State_Radiation_LowDose);
	CachedStateComp->RemoveTag(Tags.State_Radiation_AcuteSickness);
	CachedStateComp->RemoveTag(Tags.State_Radiation_CriticalARS);
	CachedStateComp->RemoveTag(Tags.State_Radiation_LeadShielded);
	CachedStateComp->RemoveTag(Tags.State_Radiation_GeigerClicking);

	if (ExposureData.LeadShieldingFactor >= 0.5f)
	{
		CachedStateComp->AddTag(Tags.State_Radiation_LeadShielded);
	}

	if (ExposureData.bIsGeigerClicking)
	{
		CachedStateComp->AddTag(Tags.State_Radiation_GeigerClicking);
	}

	switch (ExposureData.SicknessStage)
	{
	case ESBRadiationSicknessStage::MildExposure:
		CachedStateComp->AddTag(Tags.State_Radiation_Exposed);
		CachedStateComp->AddTag(Tags.State_Radiation_LowDose);
		break;
	case ESBRadiationSicknessStage::AcuteRadiationSickness:
		CachedStateComp->AddTag(Tags.State_Radiation_Exposed);
		CachedStateComp->AddTag(Tags.State_Radiation_AcuteSickness);
		break;
	case ESBRadiationSicknessStage::CriticalLethalARS:
		CachedStateComp->AddTag(Tags.State_Radiation_Exposed);
		CachedStateComp->AddTag(Tags.State_Radiation_CriticalARS);
		break;
	default:
		break;
	}
}
