#include "Components/SBStatusEffectComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"

USBStatusEffectComponent::USBStatusEffectComponent()
	: Super(FObjectInitializer::Get())
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(true);
}

void USBStatusEffectComponent::GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const
{
	FSBDebugLine HeaderLine;
	HeaderLine.Label = TEXT("Status Effects");
	HeaderLine.Value = FString::Printf(TEXT("%d active"), ActiveEffects.Entries.Num());
	HeaderLine.bIsHeader = true;
	OutDebugLines.Add(HeaderLine);

	for (const FSBStatusEffectEntry& Entry : ActiveEffects.Entries)
	{
		float Remaining = GetEffectRemainingTime(Entry.EffectTag);
		FString TimeStr = Remaining < 0.0f ? TEXT("Permanent") : FString::Printf(TEXT("%.1fs"), Remaining);

		FSBDebugLine EffectLine;
		EffectLine.Label = Entry.EffectTag.ToString();
		EffectLine.Value = TimeStr;
		EffectLine.bIsHeader = false;
		OutDebugLines.Add(EffectLine);
	}
}

bool USBStatusEffectComponent::SaveComponentData_Implementation(UObject* SavePayload)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (!Payload)
	{
		return false;
	}

	FSBSavedStatusEffectList SaveData;
	SaveData.Effects.Reserve(ActiveEffects.Entries.Num());

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	for (const FSBStatusEffectEntry& Entry : ActiveEffects.Entries)
	{
		FSBSavedStatusEffect SavedEffect;
		SavedEffect.EffectTag = Entry.EffectTag;
		SavedEffect.RemainingDuration = Entry.Duration > 0.0f ? FMath::Max(0.0f, Entry.ExpiryTime - CurrentTime) : -1.0f;
		SavedEffect.DefinitionPath = Entry.Definition ? Entry.Definition->GetPathName() : FString();
		SaveData.Effects.Add(SavedEffect);
	}

	TArray<uint8> BinaryData;
	FMemoryWriter Writer(BinaryData);
	FObjectAndNameAsStringProxyArchive Archive(Writer, true);
	Archive.ArIsSaveGame = true;

	FSBSavedStatusEffectList::StaticStruct()->SerializeItem(Archive, &SaveData, nullptr);

	Payload->WriteBinaryData(GetPathName(), BinaryData);
	return true;
}

bool USBStatusEffectComponent::LoadComponentData_Implementation(UObject* SavePayload)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (!Payload)
	{
		return false;
	}

	TArray<uint8> BinaryData;
	if (!Payload->ReadBinaryData(GetPathName(), BinaryData) || BinaryData.Num() <= 0)
	{
		return false;
	}

	FSBSavedStatusEffectList SaveData;
	FMemoryReader Reader(BinaryData);
	FObjectAndNameAsStringProxyArchive Archive(Reader, true);
	Archive.ArIsSaveGame = true;

	FSBSavedStatusEffectList::StaticStruct()->SerializeItem(Archive, &SaveData, nullptr);

	TArray<FGameplayTag> ExistingEffects;
	ExistingEffects.Reserve(ActiveEffects.Entries.Num());
	for (const FSBStatusEffectEntry& Entry : ActiveEffects.Entries)
	{
		ExistingEffects.Add(Entry.EffectTag);
	}

	for (const FGameplayTag& EffectTag : ExistingEffects)
	{
		RemoveStatusEffect(EffectTag);
	}

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	for (const FSBSavedStatusEffect& SavedEffect : SaveData.Effects)
	{
		const USBStatusEffectDefinition* Definition = Cast<USBStatusEffectDefinition>(StaticLoadObject(USBStatusEffectDefinition::StaticClass(), nullptr, *SavedEffect.DefinitionPath));
		if (!Definition || !SavedEffect.EffectTag.IsValid() || SavedEffect.RemainingDuration == 0.0f)
		{
			continue;
		}

		FSBStatusEffectEntry NewEntry;
		NewEntry.EffectTag = SavedEffect.EffectTag;
		NewEntry.Definition = Definition;
		NewEntry.Duration = SavedEffect.RemainingDuration > 0.0f ? SavedEffect.RemainingDuration : 0.0f;
		NewEntry.ExpiryTime = SavedEffect.RemainingDuration > 0.0f ? CurrentTime + SavedEffect.RemainingDuration : 0.0f;
		NewEntry.Period = Definition->DefaultPeriod;
		NewEntry.LastPeriodTriggerTime = CurrentTime;

		FSBStatusEffectEntry& AddedEntry = ActiveEffects.Entries.Add_GetRef(NewEntry);
		ActiveEffects.MarkItemDirty(AddedEntry);

		if (!CachedStateComponent)
		{
			CachedStateComponent = GetOwner()->FindComponentByClass<USBStateComponent>();
		}
		if (!CachedAttributeComponent)
		{
			CachedAttributeComponent = GetOwner()->FindComponentByClass<USBAttributeComponent>();
		}

		if (CachedStateComponent)
		{
			for (auto It = Definition->GrantedTags.CreateConstIterator(); It; ++It)
			{
				CachedStateComponent->AddTag(*It);
			}
		}

		if (CachedAttributeComponent)
		{
			for (const FSBStatusEffectModifier& ModEntry : Definition->AttributeModifiers)
			{
				FSBAttributeModifier AppliedMod = ModEntry.Modifier;
				AppliedMod.SourceTag = Definition->EffectTag;
				CachedAttributeComponent->ApplyModifier(ModEntry.AttributeTag, AppliedMod);
			}
		}
	}

	return true;
}

void USBStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedAttributeComponent = GetOwner()->FindComponentByClass<USBAttributeComponent>();
	CachedStateComponent = GetOwner()->FindComponentByClass<USBStateComponent>();
	ActiveEffects.OwnerComponent = this;
}

void USBStatusEffectComponent::ApplyStatusEffect(const USBStatusEffectDefinition* Definition)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!Definition || !Definition->EffectTag.IsValid())
	{
		return;
	}

	if (!CachedStateComponent)
	{
		CachedStateComponent = GetOwner()->FindComponentByClass<USBStateComponent>();
	}
	if (!CachedAttributeComponent)
	{
		CachedAttributeComponent = GetOwner()->FindComponentByClass<USBAttributeComponent>();
	}

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Busca se o efeito já está ativo para renovação
	FSBStatusEffectEntry* Existing = nullptr;
	for (FSBStatusEffectEntry& Entry : ActiveEffects.Entries)
	{
		if (Entry.EffectTag == Definition->EffectTag)
		{
			Existing = &Entry;
			break;
		}
	}

	if (Existing)
	{
		// Renova a expiração e reseta o trigger do período
		Existing->ExpiryTime = Definition->DefaultDuration > 0.0f ? (CurrentTime + Definition->DefaultDuration) : 0.0f;
		Existing->LastPeriodTriggerTime = CurrentTime;
		Existing->Duration = Definition->DefaultDuration;
		ActiveEffects.MarkItemDirty(*Existing);
	}
	else
	{
		// Cria nova entrada de efeito ativo
		FSBStatusEffectEntry NewEntry;
		NewEntry.EffectTag = Definition->EffectTag;
		NewEntry.ExpiryTime = Definition->DefaultDuration > 0.0f ? (CurrentTime + Definition->DefaultDuration) : 0.0f;
		NewEntry.LastPeriodTriggerTime = CurrentTime;
		NewEntry.Duration = Definition->DefaultDuration;
		NewEntry.Period = Definition->DefaultPeriod;
		NewEntry.Definition = Definition;

		ActiveEffects.Entries.Add(NewEntry);
		ActiveEffects.MarkItemDirty(ActiveEffects.Entries.Last());

		// Aplica Gameplay Tags de estado associadas
		if (CachedStateComponent)
		{
			for (auto It = Definition->GrantedTags.CreateConstIterator(); It; ++It)
			{
				CachedStateComponent->AddTag(*It);
			}
		}

		// Aplica Modificadores de Atributo associados
		if (CachedAttributeComponent)
		{
			for (const FSBStatusEffectModifier& ModEntry : Definition->AttributeModifiers)
			{
				FSBAttributeModifier AppliedMod = ModEntry.Modifier;
				AppliedMod.SourceTag = Definition->EffectTag;
				CachedAttributeComponent->ApplyModifier(ModEntry.AttributeTag, AppliedMod);
			}
		}
	}
}

void USBStatusEffectComponent::RemoveStatusEffect(FGameplayTag EffectTag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	int32 FoundIndex = INDEX_NONE;
	for (int32 i = 0; i < ActiveEffects.Entries.Num(); ++i)
	{
		if (ActiveEffects.Entries[i].EffectTag == EffectTag)
		{
			FoundIndex = i;
			break;
		}
	}

	if (FoundIndex != INDEX_NONE)
	{
		const USBStatusEffectDefinition* Def = ActiveEffects.Entries[FoundIndex].Definition;

		ActiveEffects.Entries.RemoveAt(FoundIndex);
		ActiveEffects.MarkArrayDirty();

		if (!CachedStateComponent)
		{
			CachedStateComponent = GetOwner()->FindComponentByClass<USBStateComponent>();
		}
		if (!CachedAttributeComponent)
		{
			CachedAttributeComponent = GetOwner()->FindComponentByClass<USBAttributeComponent>();
		}

		// Limpa as Gameplay Tags e Modificadores do efeito removido
		if (Def)
		{
			if (CachedStateComponent)
			{
				for (auto It = Def->GrantedTags.CreateConstIterator(); It; ++It)
				{
					CachedStateComponent->RemoveTag(*It);
				}
			}

			if (CachedAttributeComponent)
			{
				for (const FSBStatusEffectModifier& ModEntry : Def->AttributeModifiers)
				{
					CachedAttributeComponent->RemoveModifiersBySource(ModEntry.AttributeTag, Def->EffectTag);
				}
			}
		}
	}
}

bool USBStatusEffectComponent::HasStatusEffect(FGameplayTag EffectTag) const
{
	for (const FSBStatusEffectEntry& Entry : ActiveEffects.Entries)
	{
		if (Entry.EffectTag == EffectTag)
		{
			return true;
		}
	}
	return false;
}

float USBStatusEffectComponent::GetEffectRemainingTime(FGameplayTag EffectTag) const
{
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	for (const FSBStatusEffectEntry& Entry : ActiveEffects.Entries)
	{
		if (Entry.EffectTag == EffectTag)
		{
			if (Entry.Duration <= 0.0f)
			{
				return -1.0f; // Efeito permanente
			}
			return FMath::Max(0.0f, Entry.ExpiryTime - CurrentTime);
		}
	}
	return 0.0f;
}

void USBStatusEffectComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USBStatusEffectComponent, ActiveEffects);
}

void USBStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	TArray<FGameplayTag> EffectsToRemove;

	if (!CachedAttributeComponent)
	{
		CachedAttributeComponent = GetOwner()->FindComponentByClass<USBAttributeComponent>();
	}

	for (FSBStatusEffectEntry& Entry : ActiveEffects.Entries)
	{
		// 1. Processa ticks periódicos (DOT/HOT)
		if (Entry.Period > 0.0f && Entry.Definition)
		{
			while (CurrentTime - Entry.LastPeriodTriggerTime >= Entry.Period)
			{
				if (CachedAttributeComponent && Entry.Definition->PeriodAttributeTag.IsValid())
				{
					float CurrentVal = CachedAttributeComponent->GetAttributeValue(Entry.Definition->PeriodAttributeTag);
					float NewVal = CurrentVal + Entry.Definition->PeriodAttributeChange;
					CachedAttributeComponent->SetAttributeBaseValue(Entry.Definition->PeriodAttributeTag, NewVal);
				}

				Entry.LastPeriodTriggerTime += Entry.Period;
				ActiveEffects.MarkItemDirty(Entry);
			}
		}

		// 2. Registra efeitos expirados
		if (Entry.Duration > 0.0f && CurrentTime >= Entry.ExpiryTime)
		{
			EffectsToRemove.Add(Entry.EffectTag);
		}
	}

	// 3. Remove os efeitos expirados
	for (FGameplayTag Tag : EffectsToRemove)
	{
		RemoveStatusEffect(Tag);
	}
}
