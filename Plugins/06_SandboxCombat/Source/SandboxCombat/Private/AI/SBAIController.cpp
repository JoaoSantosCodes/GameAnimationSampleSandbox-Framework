#include "AI/SBAIController.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBAttributeComponent.h"
#include "SBGameplayTags.h"
#include "BrainComponent.h"
#include "SmartObjectSubsystem.h"
#include "SmartObjectRequestTypes.h"

ASBAIController::ASBAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASBAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn) return;

	CachedCombatComp = InPawn->FindComponentByClass<USBCombatComponent>();
	if (CachedCombatComp.IsValid())
	{
		CachedCombatComp->OnAgroTargetChanged.AddDynamic(this, &ASBAIController::HandleAgroTargetChanged);
		
		// Set inicial se já tiver target
		if (APawn* InitialTarget = CachedCombatComp->GetHighestAgroTarget())
		{
			HandleAgroTargetChanged(InitialTarget);
		}
	}

	CachedStateComp = InPawn->FindComponentByClass<USBStateComponent>();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->OnStateChanged.AddDynamic(this, &ASBAIController::HandleStateChanged);
		
		// Verifica se já está sob CC ao possuir
		const FSBGameplayTags& GameplayTags = FSBGameplayTags::Get();
		if (CachedStateComp->HasTag(GameplayTags.State_Character_Stunned) ||
			CachedStateComp->HasTag(GameplayTags.State_Character_Frozen))
		{
			HandleStateChanged(FGameplayTag(), true);
		}
	}

	CachedAttrComp = InPawn->FindComponentByClass<USBAttributeComponent>();
	if (CachedAttrComp.IsValid())
	{
		CachedAttrComp->OnAttributeChanged.AddDynamic(this, &ASBAIController::HandleAttributeChanged);
		SetupBossPhaseHPTracking();
	}
}

void ASBAIController::OnUnPossess()
{
	if (CachedCombatComp.IsValid())
	{
		CachedCombatComp->OnAgroTargetChanged.RemoveDynamic(this, &ASBAIController::HandleAgroTargetChanged);
	}

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->OnStateChanged.RemoveDynamic(this, &ASBAIController::HandleStateChanged);
	}

	if (CachedAttrComp.IsValid())
	{
		CachedAttrComp->OnAttributeChanged.RemoveDynamic(this, &ASBAIController::HandleAttributeChanged);
	}

	CachedCombatComp = nullptr;
	CachedStateComp = nullptr;
	CachedAttrComp = nullptr;

	Super::OnUnPossess();
}

void ASBAIController::HandleAgroTargetChanged(APawn* NewTarget)
{
	if (NewTarget)
	{
		SetFocus(NewTarget, EAIFocusPriority::Default);
	}
	else
	{
		ClearFocus(EAIFocusPriority::Default);
		ClearFocus(EAIFocusPriority::Gameplay);
	}
}

void ASBAIController::HandleStateChanged(FGameplayTag StateTag, bool bAdded)
{
	if (!CachedStateComp.IsValid()) return;

	const FSBGameplayTags& GameplayTags = FSBGameplayTags::Get();
	bool bIsStunned = CachedStateComp->HasTag(GameplayTags.State_Character_Stunned);
	bool bIsFrozen = CachedStateComp->HasTag(GameplayTags.State_Character_Frozen);

	if (bIsStunned || bIsFrozen)
	{
		StopMovement();
		ClearFocus(EAIFocusPriority::Default);
		ClearFocus(EAIFocusPriority::Gameplay);
		if (BrainComponent)
		{
			BrainComponent->PauseLogic(TEXT("StateCC"));
		}
	}
	else
	{
		// Se não está mais sob CC, resume lógica e movimento
		if (BrainComponent)
		{
			BrainComponent->ResumeLogic(TEXT("StateCC"));
		}
		
		// Volta a focar se tiver target
		if (CachedCombatComp.IsValid())
		{
			if (APawn* Target = CachedCombatComp->GetHighestAgroTarget())
			{
				SetFocus(Target, EAIFocusPriority::Default);
			}
		}
	}
}

void ASBAIController::HandleAttributeChanged(FGameplayTag AttributeTag, float NewValue, float OldValue, AActor* InInstigator)
{
	if (AttributeTag == FSBGameplayTags::Get().Attribute_Health && CachedAttrComp.IsValid())
	{
		float MaxHealth = CachedAttrComp->GetAttributeValue(FSBGameplayTags::Get().Attribute_MaxHealth);
		if (MaxHealth > 0.0f)
		{
			float NewHPPercent = NewValue / MaxHealth;
			float OldHPPercent = OldValue / MaxHealth;

			// Verifica se cruzou algum threshold
			for (int32 Index = 0; Index < BossPhaseHPThresholds.Num(); ++Index)
			{
				float Threshold = BossPhaseHPThresholds[Index];
				// Se a vida desceu abaixo do threshold nesta mudança de atributo
				if (NewHPPercent <= Threshold && OldHPPercent > Threshold)
				{
					int32 TargetPhase = Index + 1;
					if (TargetPhase > CurrentBossPhase)
					{
						CurrentBossPhase = TargetPhase;
						OnBossPhaseChanged.Broadcast(CurrentBossPhase);
					}
				}
			}
		}
	}
}

void ASBAIController::SetupBossPhaseHPTracking()
{
	CurrentBossPhase = 0;
	if (CachedAttrComp.IsValid() && BossPhaseHPThresholds.Num() > 0)
	{
		float Health = CachedAttrComp->GetAttributeValue(FSBGameplayTags::Get().Attribute_Health);
		float MaxHealth = CachedAttrComp->GetAttributeValue(FSBGameplayTags::Get().Attribute_MaxHealth);
		if (MaxHealth > 0.0f)
		{
			float HPPercent = Health / MaxHealth;
			for (int32 Index = 0; Index < BossPhaseHPThresholds.Num(); ++Index)
			{
				if (HPPercent <= BossPhaseHPThresholds[Index])
				{
					CurrentBossPhase = Index + 1;
				}
			}
		}
	}
}

bool ASBAIController::FindNearbySmartObjects(TArray<FSmartObjectRequestResult>& OutResults, FGameplayTagQuery ActivityFilter, float SearchRadius) const
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return false;

	USmartObjectSubsystem* SOSubsystem = USmartObjectSubsystem::GetCurrent(GetWorld());
	if (!SOSubsystem) return false;

	FSmartObjectRequest Request;
	Request.QueryBox = FBox::BuildAABB(MyPawn->GetActorLocation(), FVector(SearchRadius));
	Request.Filter.ActivityRequirements = ActivityFilter;

	SOSubsystem->FindSmartObjects_BP(Request, OutResults);
	return OutResults.Num() > 0;
}

bool ASBAIController::ClaimSmartObjectSlot(const FSmartObjectRequestResult& RequestResult, FSmartObjectClaimHandle& OutClaimHandle)
{
	USmartObjectSubsystem* SOSubsystem = USmartObjectSubsystem::GetCurrent(GetWorld());
	if (!SOSubsystem) return false;

	OutClaimHandle = SOSubsystem->MarkSlotAsClaimed(RequestResult.SlotHandle, ESmartObjectClaimPriority::Normal);
	return OutClaimHandle.IsValid();
}

bool ASBAIController::ReleaseSmartObjectSlot(const FSmartObjectClaimHandle& ClaimHandle)
{
	if (!ClaimHandle.IsValid()) return false;

	USmartObjectSubsystem* SOSubsystem = USmartObjectSubsystem::GetCurrent(GetWorld());
	if (!SOSubsystem) return false;

	return SOSubsystem->MarkSlotAsFree(ClaimHandle);
}

bool ASBAIController::GetSmartObjectSlotTransform(const FSmartObjectClaimHandle& ClaimHandle, FTransform& OutSlotTransform) const
{
	if (!ClaimHandle.IsValid()) return false;

	USmartObjectSubsystem* SOSubsystem = USmartObjectSubsystem::GetCurrent(GetWorld());
	if (!SOSubsystem) return false;

	TOptional<FTransform> SlotTransform = SOSubsystem->GetSlotTransform(ClaimHandle);
	if (SlotTransform.IsSet())
	{
		OutSlotTransform = SlotTransform.GetValue();
		return true;
	}
	return false;
}
