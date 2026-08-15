#include "Components/SBAttributeComponent.h"
#include "Utilities/SBLogCategories.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"

USBAttributeComponent::USBAttributeComponent()
	: Super(FObjectInitializer::Get())
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(true);
}

void USBAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USBAttributeComponent, ConfirmedPredictions);
	DOREPLIFETIME(USBAttributeComponent, PublicAttributes);
	DOREPLIFETIME_CONDITION(USBAttributeComponent, PrivateAttributes, COND_OwnerOnly);
}

#include "Subsystems/SBSaveSubsystemConcrete.h"

bool USBAttributeComponent::SaveComponentData_Implementation(UObject* SavePayload)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (Payload)
	{
		Payload->SerializeObject(GetPathName(), this);
		return true;
	}
	return false;
}

bool USBAttributeComponent::LoadComponentData_Implementation(UObject* SavePayload)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (Payload)
	{
		Payload->DeserializeObject(GetPathName(), this);

		// Sincroniza o cache local com a rede chamando ModifyAttributeBaseValue de forma limpa e autoritativa
		for (auto It = AttributesMap.CreateIterator(); It; ++It)
		{
			ModifyAttributeBaseValue(It.Key(), It.Value().BaseValue, nullptr);
		}
		return true;
	}
	return false;
}

void USBAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateModifiers(DeltaTime);
	UpdateRegeneration(DeltaTime);

	// Timeout Guard para transações pendentes no cliente
	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();
	const bool bIsLocallyControlled = GIsAutomationTesting || (Owner && Cast<APawn>(Owner) && Cast<APawn>(Owner)->IsLocallyControlled());

	if (!bIsServer && bIsLocallyControlled)
	{
		float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		for (int32 i = PendingPredictions.Num() - 1; i >= 0; --i)
		{
			// Rollback automático por timeout se expirar 2.0 segundos sem confirmação do servidor
			if (CurrentTime - PendingPredictions[i].Timestamp >= 2.0f)
			{
				FGameplayTag Tag = PendingPredictions[i].AttributeTag;
				float OldVal = GetAttributeValue(Tag);
				
				UE_LOG(LogSandboxCharacter, Warning, TEXT("Transaction PredictionId %d for Tag %s timed out! Rolling back locally."), 
					PendingPredictions[i].PredictionId, *Tag.ToString());
				
				PendingPredictions.RemoveAt(i);
				
				float NewVal = GetAttributeValue(Tag);
				if (NewVal != OldVal)
				{
					OnAttributeChanged.Broadcast(Tag, NewVal, OldVal, GetOwner());
				}
			}
		}
	}
}

void USBAttributeComponent::RegisterAttribute(FGameplayTag AttributeTag, FSBAttribute InitialValue)
{
	if (!AttributeTag.IsValid()) return;

	if (!AttributesMap.Contains(AttributeTag))
	{
		AttributesMap.Add(AttributeTag, InitialValue);
		UE_LOG(LogSandboxCharacter, Log, TEXT("Registered attribute: %s"), *AttributeTag.ToString());

		if (GetOwner() && GetOwner()->HasAuthority())
		{
			UpdateReplicatedAttribute(AttributeTag, InitialValue);
		}
	}
}

bool USBAttributeComponent::GetAttribute(FGameplayTag AttributeTag, FSBAttribute& OutAttribute) const
{
	if (const FSBAttribute* Attr = AttributesMap.Find(AttributeTag))
	{
		OutAttribute = *Attr;
		OutAttribute.CurrentValue = GetAttributeValue(AttributeTag);
		return true;
	}
	return false;
}

float USBAttributeComponent::GetAttributeValue(FGameplayTag AttributeTag) const
{
	float CurrentBase = 0.0f;
	if (const FSBAttribute* Attr = AttributesMap.Find(AttributeTag))
	{
		CurrentBase = Attr->BaseValue;
	}
	else
	{
		return 0.0f;
	}

	// Se for o cliente local (Autonomous Proxy), deduz as predições pendentes locais
	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();
	const bool bIsLocallyControlled = GIsAutomationTesting || (Owner && Cast<APawn>(Owner) && Cast<APawn>(Owner)->IsLocallyControlled());

	if (!bIsServer && bIsLocallyControlled)
	{
		float PendingOffset = 0.0f;
		for (const FSBPendingAttributePrediction& Prediction : PendingPredictions)
		{
			if (Prediction.AttributeTag == AttributeTag)
			{
				PendingOffset += Prediction.Amount;
			}
		}
		CurrentBase -= PendingOffset;
	}

	return CalculateValueWithModifiers(AttributeTag, CurrentBase);
}

bool USBAttributeComponent::TryConsumeAttribute(FGameplayTag AttributeTag, float Amount, AActor* Instigator, int32 PredictionId)
{
	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();
	const bool bIsLocallyControlled = GIsAutomationTesting || (Owner && Cast<APawn>(Owner) && Cast<APawn>(Owner)->IsLocallyControlled());

	FSBAttribute* Attr = AttributesMap.Find(AttributeTag);
	if (!Attr) return false;

	// 1. Lógica no Cliente Local (Autonomous Proxy) - Predição Transacional
	if (!bIsServer && bIsLocallyControlled)
	{
		if (PredictionId > 0)
		{
			float PredictedVal = GetAttributeValue(AttributeTag);
			if (PredictedVal >= Amount)
			{
				FSBPendingAttributePrediction NewPred;
				NewPred.AttributeTag = AttributeTag;
				NewPred.Amount = Amount;
				NewPred.PredictionId = PredictionId;
				NewPred.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
				
				PendingPredictions.Add(NewPred);

				// Broadcast local imediato de alteração do HUD
				float OldVal = PredictedVal;
				float NewVal = PredictedVal - Amount;
				OnAttributeChanged.Broadcast(AttributeTag, NewVal, OldVal, Instigator);
				return true;
			}
		}
		return false;
	}

	// 2. Lógica no Servidor (Authority) - Execução Real e Autoritativa
	if (bIsServer)
	{
		if (Attr->BaseValue >= Amount)
		{
			ModifyAttributeBaseValue(AttributeTag, Attr->BaseValue - Amount, Instigator);

			// Se o consumo veio com ID de predição, confirma no array replicado (Upsert)
			if (PredictionId > 0)
			{
				ConfirmPrediction(AttributeTag, PredictionId);
			}
			return true;
		}
	}

	return false;
}

void USBAttributeComponent::SetAttributeBaseValue(FGameplayTag AttributeTag, float NewValue)
{
	ModifyAttributeBaseValue(AttributeTag, NewValue, GetOwner());
}

void USBAttributeComponent::ApplyModifier(FGameplayTag AttributeTag, FSBAttributeModifier Modifier)
{
	if (!AttributesMap.Contains(AttributeTag)) return;

	float OldVal = GetAttributeValue(AttributeTag);
	ActiveModifiers.FindOrAdd(AttributeTag).Add(Modifier);
	float NewVal = GetAttributeValue(AttributeTag);

	if (NewVal != OldVal)
	{
		OnAttributeChanged.Broadcast(AttributeTag, NewVal, OldVal, GetOwner());
	}
}

void USBAttributeComponent::RemoveModifiersBySource(FGameplayTag AttributeTag, FGameplayTag SourceTag)
{
	if (TArray<FSBAttributeModifier>* ModList = ActiveModifiers.Find(AttributeTag))
	{
		float OldVal = GetAttributeValue(AttributeTag);
		bool bChanged = false;

		for (int32 i = ModList->Num() - 1; i >= 0; --i)
		{
			if ((*ModList)[i].SourceTag == SourceTag)
			{
				ModList->RemoveAt(i);
				bChanged = true;
			}
		}

		if (bChanged)
		{
			float NewVal = GetAttributeValue(AttributeTag);
			if (NewVal != OldVal)
			{
				OnAttributeChanged.Broadcast(AttributeTag, NewVal, OldVal, GetOwner());
			}
		}
	}
}

void USBAttributeComponent::UpdateModifiers(float DeltaTime)
{
	for (auto& Pair : ActiveModifiers)
	{
		FGameplayTag AttrTag = Pair.Key;
		TArray<FSBAttributeModifier>& ModList = Pair.Value;
		float OldVal = GetAttributeValue(AttrTag);
		bool bChanged = false;

		for (int32 i = ModList.Num() - 1; i >= 0; --i)
		{
			if (ModList[i].Duration > 0.0f)
			{
				ModList[i].Duration -= DeltaTime;
				if (ModList[i].Duration <= 0.0f)
				{
					ModList.RemoveAt(i);
					bChanged = true;
				}
			}
		}

		if (bChanged)
		{
			float NewVal = GetAttributeValue(AttrTag);
			if (NewVal != OldVal)
			{
				OnAttributeChanged.Broadcast(AttrTag, NewVal, OldVal, GetOwner());
			}
		}
	}
}

void USBAttributeComponent::UpdateRegeneration(float DeltaTime)
{
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	for (auto& Pair : AttributesMap)
	{
		FGameplayTag AttrTag = Pair.Key;
		FSBAttribute& Attr = Pair.Value;

		if (Attr.RegenRate > 0.0f && Attr.BaseValue < Attr.MaxValue)
		{
			if (CurrentTime - Attr.LastModifiedTime >= Attr.RegenDelay)
			{
				float TargetVal = Attr.BaseValue + (Attr.RegenRate * DeltaTime);
				ModifyAttributeBaseValue(AttrTag, TargetVal, GetOwner());
			}
		}
	}
}

float USBAttributeComponent::CalculateValueWithModifiers(FGameplayTag AttributeTag, float BaseVal) const
{
	float FinalValue = BaseVal;
	float AdditiveSum = 0.0f;
	float MultiplicativeMult = 1.0f;

	const TArray<FSBAttributeModifier>* ModList = ActiveModifiers.Find(AttributeTag);
	if (ModList)
	{
		for (const FSBAttributeModifier& Mod : *ModList)
		{
			switch (Mod.ModifierType)
			{
			case ESBAttributeModifierType::Override:
				return Mod.Magnitude; // Override takes precedence instantly
			case ESBAttributeModifierType::Additive:
				AdditiveSum += (Mod.Magnitude * Mod.StackCount);
				break;
			case ESBAttributeModifierType::Multiplicative:
				MultiplicativeMult *= FMath::Max(0.0f, 1.0f + (Mod.Magnitude * Mod.StackCount));
				break;
			}
		}
	}

	FinalValue += AdditiveSum;
	FinalValue *= MultiplicativeMult;

	if (const FSBAttribute* Attr = AttributesMap.Find(AttributeTag))
	{
		FinalValue = FMath::Clamp(FinalValue, Attr->MinValue, Attr->MaxValue);
	}

	return FinalValue;
}

void USBAttributeComponent::ConfirmPrediction(FGameplayTag AttributeTag, int32 PredictionId)
{
	if (GetOwner() && !GetOwner()->HasAuthority() && !GIsAutomationTesting) return;

	// Upsert no array de confirmações: busca se a tag já existe
	bool bFound = false;
	for (FSBConfirmedPredictionEntry& Entry : ConfirmedPredictions)
	{
		if (Entry.AttributeTag == AttributeTag)
		{
			// Atualiza o ID apenas se for maior (FIFO / Rajada)
			if (PredictionId > Entry.ConfirmedPredictionId)
			{
				Entry.ConfirmedPredictionId = PredictionId;
			}
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		FSBConfirmedPredictionEntry NewEntry;
		NewEntry.AttributeTag = AttributeTag;
		NewEntry.ConfirmedPredictionId = PredictionId;
		ConfirmedPredictions.Add(NewEntry);
	}
}

void USBAttributeComponent::ClientRollbackPrediction_Implementation(FGameplayTag AttributeTag, int32 PredictionId)
{
	// Remove a transação rejeitada da fila do cliente
	for (int32 i = PendingPredictions.Num() - 1; i >= 0; --i)
	{
		if (PendingPredictions[i].AttributeTag == AttributeTag && PendingPredictions[i].PredictionId == PredictionId)
		{
			float OldVal = GetAttributeValue(AttributeTag);
			PendingPredictions.RemoveAt(i);
			float NewVal = GetAttributeValue(AttributeTag);
			
			// Notifica o HUD de que o recurso foi restaurado
			if (NewVal != OldVal)
			{
				OnAttributeChanged.Broadcast(AttributeTag, NewVal, OldVal, GetOwner());
			}
			break;
		}
	}
}

void USBAttributeComponent::OnRep_ConfirmedPredictions()
{
	CleanPendingPredictionsAndNotify();
}

void USBAttributeComponent::OnRep_PublicAttributes()
{
	// Desempacota o array de replicação de volta no mapa de atributos local do cliente
	for (const FSBAttributeReplicationEntry& Entry : PublicAttributes)
	{
		FSBAttribute* Attr = AttributesMap.Find(Entry.Tag);
		float OldVal = GetAttributeValue(Entry.Tag);

		if (Attr)
		{
			*Attr = Entry.Attribute;
		}
		else
		{
			AttributesMap.Add(Entry.Tag, Entry.Attribute);
		}

		float NewVal = GetAttributeValue(Entry.Tag);
		if (NewVal != OldVal)
		{
			OnAttributeChanged.Broadcast(Entry.Tag, NewVal, OldVal, GetOwner());
		}
	}

	CleanPendingPredictionsAndNotify();
}

void USBAttributeComponent::OnRep_PrivateAttributes()
{
	// Desempacota o array de replicação de volta no mapa de atributos local do cliente
	for (const FSBAttributeReplicationEntry& Entry : PrivateAttributes)
	{
		FSBAttribute* Attr = AttributesMap.Find(Entry.Tag);
		float OldVal = GetAttributeValue(Entry.Tag);

		if (Attr)
		{
			*Attr = Entry.Attribute;
		}
		else
		{
			AttributesMap.Add(Entry.Tag, Entry.Attribute);
		}

		float NewVal = GetAttributeValue(Entry.Tag);
		if (NewVal != OldVal)
		{
			OnAttributeChanged.Broadcast(Entry.Tag, NewVal, OldVal, GetOwner());
		}
	}

	CleanPendingPredictionsAndNotify();
}

bool USBAttributeComponent::IsAttributePrivate(FGameplayTag Tag) const
{
	FString TagName = Tag.ToString();
	return TagName.Contains(TEXT("Mana")) || TagName.Contains(TEXT("Stamina")) || TagName.Contains(TEXT("Ammo")) || TagName.Contains(TEXT("XP"));
}

void USBAttributeComponent::OnRep_ReplicatedAttributes()
{
	OnRep_PublicAttributes();
	OnRep_PrivateAttributes();
}

void USBAttributeComponent::UpdateReplicatedAttribute(FGameplayTag Tag, const FSBAttribute& Attr)
{
	if (GetOwner() && !GetOwner()->HasAuthority() && !GIsAutomationTesting) return;

	const bool bIsPrivate = IsAttributePrivate(Tag);
	TArray<FSBAttributeReplicationEntry>& TargetArray = bIsPrivate ? PrivateAttributes : PublicAttributes;

	// Upsert no array de replicação de atributos correto
	bool bFound = false;
	for (FSBAttributeReplicationEntry& Entry : TargetArray)
	{
		if (Entry.Tag == Tag)
		{
			Entry.Attribute = Attr;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		FSBAttributeReplicationEntry NewEntry;
		NewEntry.Tag = Tag;
		NewEntry.Attribute = Attr;
		TargetArray.Add(NewEntry);
	}
}

void USBAttributeComponent::ModifyAttributeBaseValue(FGameplayTag Tag, float NewValue, AActor* Instigator)
{
	FSBAttribute* Attr = AttributesMap.Find(Tag);
	if (!Attr) return;

	float OldVal = GetAttributeValue(Tag);
	Attr->BaseValue = FMath::Clamp(NewValue, Attr->MinValue, Attr->MaxValue);
	Attr->LastModifiedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	if (GetOwner() && (GetOwner()->HasAuthority() || GIsAutomationTesting))
	{
		UpdateReplicatedAttribute(Tag, *Attr);
	}

	float NewVal = GetAttributeValue(Tag);
	if (NewVal != OldVal)
	{
		OnAttributeChanged.Broadcast(Tag, NewVal, OldVal, Instigator);
	}
}

void USBAttributeComponent::CleanPendingPredictionsAndNotify()
{
	// Varre as confirmações replicadas e remove da fila local do cliente
	for (const FSBConfirmedPredictionEntry& Entry : ConfirmedPredictions)
	{
		for (int32 i = PendingPredictions.Num() - 1; i >= 0; --i)
		{
			if (PendingPredictions[i].AttributeTag == Entry.AttributeTag && PendingPredictions[i].PredictionId <= Entry.ConfirmedPredictionId)
			{
				float OldVal = GetAttributeValue(Entry.AttributeTag);
				PendingPredictions.RemoveAt(i);
				float NewVal = GetAttributeValue(Entry.AttributeTag);

				// Se o valor real mudou após limpar o offset local, notifica a UI/HUD
				if (NewVal != OldVal)
				{
					OnAttributeChanged.Broadcast(Entry.AttributeTag, NewVal, OldVal, GetOwner());
				}
			}
		}
	}
}

void USBAttributeComponent::GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const
{
	FSBDebugLine Header;
	Header.Label = GetClass()->GetName();
	Header.bIsHeader = true;
	OutDebugLines.Add(Header);

	// Attributes
	FSBDebugLine AttrHeader;
	AttrHeader.Label = TEXT("Attributes Registered");
	AttrHeader.bIsHeader = true;
	OutDebugLines.Add(AttrHeader);

	for (const auto& Pair : AttributesMap)
	{
		FGameplayTag Tag = Pair.Key;
		const FSBAttribute& Attr = Pair.Value;

		FSBDebugLine Line;
		Line.Label = Tag.ToString();
		Line.Value = FString::Printf(TEXT("Base: %.1f | Current: %.1f | Modifiers: %d"),
			Attr.BaseValue, GetAttributeValue(Tag), ActiveModifiers.FindRef(Tag).Num());
		OutDebugLines.Add(Line);
	}

	// Pending Predictions (Client-only / Local)
	if (PendingPredictions.Num() > 0)
	{
		FSBDebugLine PredHeader;
		PredHeader.Label = FString::Printf(TEXT("Pending Predictions (%d)"), PendingPredictions.Num());
		PredHeader.bIsHeader = true;
		OutDebugLines.Add(PredHeader);

		for (const FSBPendingAttributePrediction& Pred : PendingPredictions)
		{
			FSBDebugLine Line;
			Line.Label = FString::Printf(TEXT("  Id: %d"), Pred.PredictionId);
			Line.Value = FString::Printf(TEXT("%s Amount: %.1f"), *Pred.AttributeTag.ToString(), Pred.Amount);
			OutDebugLines.Add(Line);
		}
	}
}

void USBAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	OnAttributeChanged.AddDynamic(this, &USBAttributeComponent::HandleAttributeChangedInternal);
}

void USBAttributeComponent::HandleAttributeChangedInternal(FGameplayTag AttributeTag, float NewValue, float OldValue, AActor* Instigator)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USBEventSubsystem* EventSubsystem = GI->GetSubsystem<USBEventSubsystem>())
			{
				USBAttributeChangedPayload* Payload = NewObject<USBAttributeChangedPayload>(this);
				Payload->TargetPawn = Cast<APawn>(GetOwner());
				Payload->AttributeTag = AttributeTag;
				float BaseVal = 0.0f;
				float MaxVal = 0.0f;
				if (const FSBAttribute* Attr = AttributesMap.Find(AttributeTag))
				{
					BaseVal = Attr->BaseValue;
					MaxVal = Attr->MaxValue;
				}
				Payload->BaseValue = BaseVal;
				Payload->CurrentValue = NewValue;
				Payload->MaxValue = MaxVal;

				EventSubsystem->PublishEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Attribute.Changed")), Payload);
			}
		}
	}
}
