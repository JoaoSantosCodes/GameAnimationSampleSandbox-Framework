#include "Components/SBAbilityComponent.h"
#include "Abilities/SBAbility.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Interfaces/SBCharacterInterface.h"
#include "DataAssets/SBPawnDataAsset.h"
#include "DataAssets/SBAbilitySetDataAsset.h"
#include "Input/SBInputComponent.h"
#include "Input/SBInputConfig.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/SBLogCategories.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"

USBAbilityComponent::USBAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	CooldownsList.OwnerComponent = this;
}

void USBAbilityComponent::OnInitialize_Implementation()
{
	Super::OnInitialize_Implementation();
	CooldownsList.OwnerComponent = this;
}

void USBAbilityComponent::OnPostInitialize_Implementation()
{
	Super::OnPostInitialize_Implementation();

	if (!GetOwner()) return;

	ISBCharacterInterface* CharInterface = Cast<ISBCharacterInterface>(GetOwner());
	if (CharInterface)
	{
		UObject* RawPawnData = CharInterface->GetPawnData_Implementation();
		if (USBPawnDataAsset* PawnData = Cast<USBPawnDataAsset>(RawPawnData))
		{
			if (PawnData->AbilitySet)
			{
				for (const FSBAbilitySetEntry& Entry : PawnData->AbilitySet->Abilities)
				{
					if (Entry.AbilityClass)
					{
						TSubclassOf<USBAbility> AbilityClass = Cast<UClass>(Entry.AbilityClass);
						if (AbilityClass)
						{
							USBAbility* Granted = GrantAbility(AbilityClass, Entry.Definition);
							if (Granted)
							{
								if (Entry.AbilityTag.IsValid())
								{
									Granted->AbilityTag = Entry.AbilityTag;
								}
								if (Entry.InputTag.IsValid())
								{
									InputToAbilityMap.Add(Entry.InputTag, Granted->AbilityTag);
								}
							}
						}
					}
				}
			}
		}
	}
}

USBAbility* USBAbilityComponent::GrantAbility(TSubclassOf<USBAbility> AbilityClass, USBGameplayBehaviorDefinition* Definition)
{
	if (!AbilityClass) return nullptr;

	USBAbility* NewAbility = NewObject<USBAbility>(GetOwner(), AbilityClass);
	if (NewAbility)
	{
		NewAbility->Initialize(this, Definition);
		AddAvailableBehavior(NewAbility);
		UE_LOG(LogSandboxCharacter, Log, TEXT("Granted ability: %s"), *NewAbility->GetName());
		return NewAbility;
	}
	return nullptr;
}

bool USBAbilityComponent::ActivateAbilityByTag(FGameplayTag AbilityTag)
{
	return RequestBehavior(AbilityTag);
}

void USBAbilityComponent::EndAbilityByTag(FGameplayTag AbilityTag)
{
	StopBehavior(AbilityTag);
}

void USBAbilityComponent::BindInputActions(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent) return;

	USBInputComponent* SBInputComp = Cast<USBInputComponent>(PlayerInputComponent);
	if (!SBInputComp) return;

	ISBCharacterInterface* CharInterface = Cast<ISBCharacterInterface>(GetOwner());
	if (!CharInterface) return;

	UObject* RawPawnData = CharInterface->GetPawnData_Implementation();
	USBPawnDataAsset* PawnData = Cast<USBPawnDataAsset>(RawPawnData);
	if (!PawnData || !PawnData->AbilitySet || !PawnData->InputConfig) return;

	USBInputConfig* SBInputConfig = Cast<USBInputConfig>(PawnData->InputConfig);
	if (!SBInputConfig) return;

	for (const FSBAbilitySetEntry& Entry : PawnData->AbilitySet->Abilities)
	{
		if (Entry.InputTag.IsValid())
		{
			SBInputComp->BindActionByTag(SBInputConfig, Entry.InputTag, ETriggerEvent::Started, this, &USBAbilityComponent::Input_AbilityInputPressed);
			SBInputComp->BindActionByTag(SBInputConfig, Entry.InputTag, ETriggerEvent::Completed, this, &USBAbilityComponent::Input_AbilityInputReleased);
		}
	}
}

bool USBAbilityComponent::IsAbilityOnCooldown(FGameplayTag AbilityTag) const
{
	if (!AbilityTag.IsValid()) return false;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	const FSBCooldownEntry* Entry = CooldownsList.Entries.FindByPredicate([&AbilityTag](const FSBCooldownEntry& E) {
		return E.AbilityTag == AbilityTag;
	});
	if (Entry)
	{
		return CurrentTime < Entry->ExpiryTime;
	}
	return false;
}

float USBAbilityComponent::GetRemainingCooldownTime(FGameplayTag AbilityTag) const
{
	if (!AbilityTag.IsValid()) return 0.0f;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	const FSBCooldownEntry* Entry = CooldownsList.Entries.FindByPredicate([&AbilityTag](const FSBCooldownEntry& E) {
		return E.AbilityTag == AbilityTag;
	});
	if (Entry)
	{
		return FMath::Max(0.0f, Entry->ExpiryTime - CurrentTime);
	}
	return 0.0f;
}

bool USBAbilityComponent::RequestBehavior(FGameplayTag BehaviorTag)
{
	if (IsAbilityOnCooldown(BehaviorTag))
	{
		UE_LOG(LogSandboxCharacter, Warning, TEXT("Ability %s is on cooldown!"), *BehaviorTag.ToString());
		return false;
	}

	USBGameplayBehavior* Behavior = FindAvailableBehaviorByTag(BehaviorTag);
	USBAbility* Ability = Cast<USBAbility>(Behavior);
	if (!Ability)
	{
		return Super::RequestBehavior(BehaviorTag);
	}

	int32 PredictionId = 0;
	if (GetOwnerRole() == ROLE_Authority)
	{
		PredictionId = CurrentServerPredictionId;
	}
	else
	{
		PredictionId = ++LocalPredictionId;
	}

	bool bResourceConsumed = false;

	if (Ability->ResourceTag.IsValid() && Ability->ResourceCost > 0.0f)
	{
		ISBCharacterInterface* CharInterface = Cast<ISBCharacterInterface>(GetOwner());
		if (USBAttributeComponent* AttribComp = CharInterface ? Cast<USBAttributeComponent>(CharInterface->GetAttributeComponent_Implementation()) : nullptr)
		{
			bResourceConsumed = AttribComp->TryConsumeAttribute(Ability->ResourceTag, Ability->ResourceCost, GetOwner(), PredictionId);
			if (!bResourceConsumed)
			{
				UE_LOG(LogSandboxCharacter, Warning, TEXT("Resource consumption failed for ability %s"), *BehaviorTag.ToString());
				return false;
			}
			else if (Ability->ResourceTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Attribute.Mana"))))
			{
				if (UWorld* World = GetWorld())
				{
					LastManaConsumptionTime = World->GetTimeSeconds();
				}
			}
		}
	}

	bool bStackSuccess = Super::RequestBehavior(BehaviorTag);
	if (bStackSuccess)
	{
		if (Ability->CooldownDuration > 0.0f)
		{
			float ExpiryTime = GetWorld()->GetTimeSeconds() + Ability->CooldownDuration;
			FSBCooldownEntry* Existing = CooldownsList.Entries.FindByPredicate([&BehaviorTag](const FSBCooldownEntry& E) {
				return E.AbilityTag == BehaviorTag;
			});
			if (Existing)
			{
				Existing->ExpiryTime = ExpiryTime;
				CooldownsList.MarkItemDirty(*Existing);
			}
			else
			{
				FSBCooldownEntry NewEntry;
				NewEntry.AbilityTag = BehaviorTag;
				NewEntry.ExpiryTime = ExpiryTime;
				CooldownsList.Entries.Add(NewEntry);
				CooldownsList.MarkArrayDirty();
			}

			if (Ability->CooldownTag.IsValid())
			{
				if (USBStateComponent* StateComp = GetOwner() ? GetOwner()->FindComponentByClass<USBStateComponent>() : nullptr)
				{
					StateComp->AddTag(Ability->CooldownTag);
				}
			}

			if (UWorld* World = GetWorld())
			{
				if (UGameInstance* GI = World->GetGameInstance())
				{
					if (USBEventSubsystem* EventSubsystem = GI->GetSubsystem<USBEventSubsystem>())
					{
						USBCooldownEventPayload* CooldownPayload = NewObject<USBCooldownEventPayload>(this);
						CooldownPayload->TargetPawn = Cast<APawn>(GetOwner());
						CooldownPayload->AbilityTag = BehaviorTag;
						CooldownPayload->Duration = Ability->CooldownDuration;

						EventSubsystem->PublishEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Ability.CooldownStarted")), CooldownPayload);
					}
				}
			}
		}

		if (GetOwnerRole() == ROLE_AutonomousProxy)
		{
			ServerRequestBehavior(BehaviorTag, PredictionId);
		}

		return true;
	}

	if (bResourceConsumed)
	{
		ISBCharacterInterface* CharInterface = Cast<ISBCharacterInterface>(GetOwner());
		if (USBAttributeComponent* AttribComp = CharInterface ? Cast<USBAttributeComponent>(CharInterface->GetAttributeComponent_Implementation()) : nullptr)
		{
			if (GetOwnerRole() == ROLE_Authority)
			{
				FSBAttribute Attr;
				if (AttribComp->GetAttribute(Ability->ResourceTag, Attr))
				{
					AttribComp->SetAttributeBaseValue(Ability->ResourceTag, Attr.BaseValue + Ability->ResourceCost);
				}
			}
			else
			{
				AttribComp->ClientRollbackPrediction(Ability->ResourceTag, PredictionId);
			}
		}
	}

	return false;
}

void USBAbilityComponent::StopBehavior(FGameplayTag BehaviorTag, bool bSkipServerNotify, bool bSkipClientNotify)
{
	Super::StopBehavior(BehaviorTag, bSkipServerNotify, bSkipClientNotify);

	if (GetOwnerRole() == ROLE_AutonomousProxy && !bSkipServerNotify)
	{
		ServerStopBehavior(BehaviorTag);
	}
}

void USBAbilityComponent::ServerRequestBehavior_Implementation(FGameplayTag BehaviorTag, int32 PredictionId)
{
	if (IsAbilityOnCooldown(BehaviorTag))
	{
		ClientRollbackAbility(BehaviorTag, PredictionId);
		return;
	}

	CurrentServerPredictionId = PredictionId;
	bool bSuccess = RequestBehavior(BehaviorTag);
	CurrentServerPredictionId = 0;

	if (!bSuccess)
	{
		ClientRollbackAbility(BehaviorTag, PredictionId);
	}
}

bool USBAbilityComponent::ServerRequestBehavior_Validate(FGameplayTag BehaviorTag, int32 PredictionId)
{
	if (!AbilityRPCLimiter.AllowRPC(GetWorld(), 20.0f))
	{
		return false;
	}

	if (!FindAvailableBehaviorByTag(BehaviorTag))
	{
		return false;
	}

	return true;
}

void USBAbilityComponent::ServerStopBehavior_Implementation(FGameplayTag BehaviorTag)
{
	StopBehavior(BehaviorTag, true, false);
}

bool USBAbilityComponent::ServerStopBehavior_Validate(FGameplayTag BehaviorTag)
{
	if (!AbilityRPCLimiter.AllowRPC(GetWorld(), 20.0f))
	{
		return false;
	}
	return true;
}

void USBAbilityComponent::ClientStopBehavior_Implementation(FGameplayTag BehaviorTag)
{
	StopBehavior(BehaviorTag, true, true);
}

void USBAbilityComponent::ClientRollbackAbility_Implementation(FGameplayTag BehaviorTag, int32 PredictionId)
{
	StopBehavior(BehaviorTag, true, false);

	USBGameplayBehavior* Behavior = FindAvailableBehaviorByTag(BehaviorTag);
	if (USBAbility* Ability = Cast<USBAbility>(Behavior))
	{
		if (Ability->ResourceTag.IsValid())
		{
			ISBCharacterInterface* CharInterface = Cast<ISBCharacterInterface>(GetOwner());
			if (USBAttributeComponent* AttribComp = CharInterface ? Cast<USBAttributeComponent>(CharInterface->GetAttributeComponent_Implementation()) : nullptr)
			{
				AttribComp->ClientRollbackPrediction(Ability->ResourceTag, PredictionId);
			}
		}

		if (Ability->CooldownDuration > 0.0f)
		{
			int32 Index = CooldownsList.Entries.IndexOfByPredicate([&BehaviorTag](const FSBCooldownEntry& E) {
				return E.AbilityTag == BehaviorTag;
			});
			if (Index != INDEX_NONE)
			{
				CooldownsList.Entries.RemoveAt(Index);
				CooldownsList.MarkArrayDirty();
			}

			if (Ability->CooldownTag.IsValid())
			{
				if (USBStateComponent* StateComp = GetOwner() ? GetOwner()->FindComponentByClass<USBStateComponent>() : nullptr)
				{
					StateComp->RemoveTag(Ability->CooldownTag);
				}
			}
		}
	}
}

void USBAbilityComponent::OnBehaviorEjected(FGameplayTag BehaviorTag, bool bSkipServerNotify, bool bSkipClientNotify)
{
	Super::OnBehaviorEjected(BehaviorTag, bSkipServerNotify, bSkipClientNotify);

	if (GetOwnerRole() == ROLE_Authority && !bSkipClientNotify)
	{
		ClientStopBehavior(BehaviorTag);
	}
}

void USBAbilityComponent::Input_AbilityInputPressed(FGameplayTag InputTag)
{
	if (const FGameplayTag* AbilityTag = InputToAbilityMap.Find(InputTag))
	{
		ActivateAbilityByTag(*AbilityTag);
	}
}

void USBAbilityComponent::Input_AbilityInputReleased(FGameplayTag InputTag)
{
	if (const FGameplayTag* AbilityTag = InputToAbilityMap.Find(InputTag))
	{
		EndAbilityByTag(*AbilityTag);
	}
}

void USBAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USBAbilityComponent, CooldownsList);
}

void USBAbilityComponent::GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const
{
	Super::GetDebugDescription_Implementation(OutDebugLines);

	FSBDebugLine Header;
	Header.Label = TEXT("Ability Component Telemetry");
	Header.bIsHeader = true;
	OutDebugLines.Add(Header);

	FSBDebugLine PredLine;
	PredLine.Label = TEXT("Prediction IDs");
	PredLine.Value = FString::Printf(TEXT("Local: %d | Server Active: %d"), LocalPredictionId, CurrentServerPredictionId);
	OutDebugLines.Add(PredLine);

	// Granted Abilities
	FSBDebugLine AbilitiesHeader;
	AbilitiesHeader.Label = TEXT("Granted Abilities Mapping");
	AbilitiesHeader.bIsHeader = true;
	OutDebugLines.Add(AbilitiesHeader);

	for (const auto& Pair : InputToAbilityMap)
	{
		FSBDebugLine Line;
		Line.Label = Pair.Key.ToString();
		Line.Value = Pair.Value.ToString();
		OutDebugLines.Add(Line);
	}

	// Cooldowns
	FSBDebugLine CooldownsHeader;
	CooldownsHeader.Label = FString::Printf(TEXT("Active Cooldowns (%d)"), CooldownsList.Entries.Num());
	CooldownsHeader.bIsHeader = true;
	OutDebugLines.Add(CooldownsHeader);

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	for (const FSBCooldownEntry& Entry : CooldownsList.Entries)
	{
		FSBDebugLine Line;
		Line.Label = Entry.AbilityTag.ToString();
		Line.Value = FString::Printf(TEXT("Remaining: %.1fs"), FMath::Max(0.f, Entry.ExpiryTime - CurrentTime));
		OutDebugLines.Add(Line);
	}
}

void USBAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UWorld* World = GetWorld())
	{
		float CurrentTime = World->GetTimeSeconds();
		for (int32 i = CooldownsList.Entries.Num() - 1; i >= 0; --i)
		{
			if (CurrentTime >= CooldownsList.Entries[i].ExpiryTime)
			{
				FGameplayTag ExpiredTag = CooldownsList.Entries[i].AbilityTag;

				USBGameplayBehavior* Behavior = FindAvailableBehaviorByTag(ExpiredTag);
				if (USBAbility* Ability = Cast<USBAbility>(Behavior))
				{
					if (Ability->CooldownTag.IsValid())
					{
						if (USBStateComponent* StateComp = GetOwner() ? GetOwner()->FindComponentByClass<USBStateComponent>() : nullptr)
						{
							StateComp->RemoveTag(Ability->CooldownTag);
						}
					}
				}

				CooldownsList.Entries.RemoveAt(i);
				CooldownsList.MarkArrayDirty();

				// Publish CooldownEnded
				if (UGameInstance* GI = World->GetGameInstance())
				{
					if (USBEventSubsystem* EventSubsystem = GI->GetSubsystem<USBEventSubsystem>())
					{
						USBCooldownEventPayload* CooldownPayload = NewObject<USBCooldownEventPayload>(this);
						CooldownPayload->TargetPawn = Cast<APawn>(GetOwner());
						CooldownPayload->AbilityTag = ExpiredTag;
						CooldownPayload->Duration = 0.0f;

						EventSubsystem->PublishEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Ability.CooldownEnded")), CooldownPayload);
					}
				}
			}
		}

		// Passive Mana Regeneration (Authority-only)
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			if (CurrentTime - LastManaConsumptionTime >= ManaRegenDelay)
			{
				ISBCharacterInterface* CharInterface = Cast<ISBCharacterInterface>(GetOwner());
				if (USBAttributeComponent* AttribComp = CharInterface ? Cast<USBAttributeComponent>(CharInterface->GetAttributeComponent_Implementation()) : nullptr)
				{
					FGameplayTag ManaTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Mana"), false);
					FSBAttribute ManaAttr;
					if (ManaTag.IsValid() && AttribComp->GetAttribute(ManaTag, ManaAttr))
					{
						if (ManaAttr.BaseValue < ManaAttr.MaxValue)
						{
							float NewMana = FMath::Min(ManaAttr.MaxValue, ManaAttr.BaseValue + (ManaRegenRate * DeltaTime));
							AttribComp->SetAttributeBaseValue(ManaTag, NewMana);
						}
					}
				}
			}
		}
	}
}
