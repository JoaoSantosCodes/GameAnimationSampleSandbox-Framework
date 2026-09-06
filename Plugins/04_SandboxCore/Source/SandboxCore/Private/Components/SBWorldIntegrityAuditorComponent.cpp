#include "Components/SBWorldIntegrityAuditorComponent.h"
#include "Subsystems/SBWorldIntegritySubsystem.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USBWorldIntegrityAuditorComponent::USBWorldIntegrityAuditorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBWorldIntegrityAuditorComponent::OnInitialize_Implementation()
{
	RunAudit();
}

void USBWorldIntegrityAuditorComponent::OnReady_Implementation()
{
	RunAudit();
}

void USBWorldIntegrityAuditorComponent::OnShutdown_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Integrity_Audited);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Integrity_IssueDetected);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Integrity_Clean);
		}
	}
}

bool USBWorldIntegrityAuditorComponent::RunAudit()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	bAuditPassed = true;
	if (UWorld* World = GetWorld())
	{
		if (USBWorldIntegritySubsystem* Subsystem = World->GetSubsystem<USBWorldIntegritySubsystem>())
		{
			FSBWorldIntegrityReport Report = Subsystem->GetLastReport();
			bAuditPassed = Report.bPassed;
		}
	}

	SyncTags();
	return bAuditPassed;
}

void USBWorldIntegrityAuditorComponent::InjectTestIssue(FName IssueId, const FString& Description, ESBIntegritySeverity Severity, bool bAutoFixable)
{
	if (UWorld* World = GetWorld())
	{
		if (USBWorldIntegritySubsystem* Subsystem = World->GetSubsystem<USBWorldIntegritySubsystem>())
		{
			Subsystem->RegisterIssue(FSBIntegrityIssue(IssueId, GetOwner() ? GetOwner()->GetName() : TEXT("Auditor"), Description, Severity, bAutoFixable));
			Subsystem->GenerateReport();
			RunAudit();
		}
	}
}

void USBWorldIntegrityAuditorComponent::SyncTags()
{
	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Integrity_Audited);
			if (bAuditPassed)
			{
				ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Integrity_Clean);
				ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Integrity_IssueDetected);
			}
			else
			{
				ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Integrity_Clean);
				ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Integrity_IssueDetected);
			}
		}
	}
}
