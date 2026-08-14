#include "Components/SBCombatComponent.h"
#include "Weapons/SBWeaponBehavior.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Interfaces/SBEquippableInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Subsystems/SBEventSubsystem.h"
#include "UObject/UnrealType.h"
#include "Engine/GameInstance.h"

USBCombatComponent::USBCombatComponent()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBCombatComponent::OnInitialize_Implementation()
{
	Super::OnInitialize_Implementation();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		CachedAttributeComponent = Owner->FindComponentByClass<USBAttributeComponent>();
		CachedStateComponent = Owner->FindComponentByClass<USBStateComponent>();
	}

	if (DefaultCombatConfig)
	{
		LoadCombatConfig(DefaultCombatConfig);
	}

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USBEventSubsystem* EventSubsystem = GI->GetSubsystem<USBEventSubsystem>())
			{
				EventSubsystem->SubscribeToEventNative(
					FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.ItemEquipped")),
					ESBEventPriority::Medium,
					FSBNativeEventDelegate::CreateUObject(this, &USBCombatComponent::OnItemEquipped)
				);
				EventSubsystem->SubscribeToEventNative(
					FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.ItemUnequipped")),
					ESBEventPriority::Medium,
					FSBNativeEventDelegate::CreateUObject(this, &USBCombatComponent::OnItemUnequipped)
				);
			}
		}
	}
}

void USBCombatComponent::OnShutdown_Implementation()
{
	Super::OnShutdown_Implementation();
	LastExecutionTimes.Empty();
}

TArray<TObjectPtr<USBWeaponBehavior>> USBCombatComponent::GetActiveWeapons() const
{
	TArray<TObjectPtr<USBWeaponBehavior>> Results;
	for (USBGameplayBehavior* Behavior : ActiveBehaviors)
	{
		if (USBWeaponBehavior* Weapon = Cast<USBWeaponBehavior>(Behavior))
		{
			Results.Add(Weapon);
		}
	}
	return Results;
}

TArray<TObjectPtr<USBWeaponBehavior>> USBCombatComponent::GetAvailableWeapons() const
{
	TArray<TObjectPtr<USBWeaponBehavior>> Results;
	for (USBGameplayBehavior* Behavior : AvailableBehaviors)
	{
		if (USBWeaponBehavior* Weapon = Cast<USBWeaponBehavior>(Behavior))
		{
			Results.Add(Weapon);
		}
	}
	return Results;
}

void USBCombatComponent::LoadCombatConfig(USBCombatConfigDataAsset* NewConfig)
{
	if (!NewConfig) return;

	// Limpa armas ativas
	FSBGameplayContext GameplayCtx;
	FSBFrameworkContext FrameworkCtx;
	FSBBehaviorContext Context = BuildBehaviorContext(0.f, GameplayCtx, FrameworkCtx);

	for (USBGameplayBehavior* Active : ActiveBehaviors)
	{
		if (Active)
		{
			Active->Exit(Context);
		}
	}
	ActiveBehaviors.Empty();
	AvailableBehaviors.Empty();

	// Instancia os behaviors configurados
	for (const FSBWeaponConfigEntry& Entry : NewConfig->ConfiguredWeapons)
	{
		if (Entry.BehaviorClass && Entry.DefinitionAsset)
		{
			USBWeaponBehavior* NewBehavior = NewObject<USBWeaponBehavior>(this, Entry.BehaviorClass);
			NewBehavior->Initialize(this, Entry.DefinitionAsset);
			AvailableBehaviors.Add(NewBehavior);
		}
	}
}

bool USBCombatComponent::RequestWeaponBehavior(FGameplayTag BehaviorTag, int32 PredictionId)
{
	if (!BehaviorTag.IsValid()) return false;

	USBWeaponBehavior* TargetWeapon = FindAvailableWeaponByTag(BehaviorTag);
	if (!TargetWeapon) return false;

	USBWeaponBehaviorDefinition* Def = TargetWeapon->GetDefinition();
	if (!Def) return false;

	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();
	const bool bIsLocallyControlled = GIsAutomationTesting || (Owner && Cast<APawn>(Owner) && Cast<APawn>(Owner)->IsLocallyControlled());

	if (!bIsServer && !bIsLocallyControlled) return false;

	FSBBehaviorStackMutationGuard Guard(this); // Proteção contra reentrância recursiva na base

	// 1. Verifica Cooldown (Taxa de Disparo)
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (float* LastTime = LastExecutionTimes.Find(BehaviorTag))
	{
		if (CurrentTime - *LastTime < Def->FireRate)
		{
			return false;
		}
	}

	FSBGameplayContext GameplayCtx;
	FSBFrameworkContext FrameworkCtx;
	FSBBehaviorContext Context = BuildBehaviorContext(0.f, GameplayCtx, FrameworkCtx);

	// 2. Verifica se pode entrar (Tags e Recursos preditos)
	if (!TargetWeapon->CanEnter(Context))
	{
		return false;
	}

	// 3. Consome Atributos (Munição/Mana)
	if (CachedAttributeComponent)
	{
		if (Def->AmmoCost > 0.0f)
		{
			FGameplayTag AmmoTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Weapon.Ammo"));
			bool bAmmoConsumed = CachedAttributeComponent->TryConsumeAttribute(AmmoTag, Def->AmmoCost, Owner, PredictionId);
			if (!bAmmoConsumed)
			{
				return false;
			}
		}

		if (Def->ManaCost > 0.0f)
		{
			FGameplayTag ManaTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Mana"));
			bool bManaConsumed = CachedAttributeComponent->TryConsumeAttribute(ManaTag, Def->ManaCost, Owner, PredictionId);
			if (!bManaConsumed)
			{
				return false;
			}
		}
	}

	// 4. Executa a ativação através da pilha comum
	if (RequestBehavior(BehaviorTag))
	{
		// Registra o tempo do último disparo/ativação
		LastExecutionTimes.FindOrAdd(BehaviorTag) = CurrentTime;

		// 5. Envia RPC para o Servidor se for cliente
		if (!bIsServer && bIsLocallyControlled)
		{
			ServerRequestFire(BehaviorTag, PredictionId);
		}
		return true;
	}

	return false;
}

void USBCombatComponent::StopWeaponBehavior(FGameplayTag BehaviorTag, bool bSkipServerNotify)
{
	StopBehavior(BehaviorTag, bSkipServerNotify, false);
}

bool USBCombatComponent::HasWeaponBehavior(FGameplayTag BehaviorTag) const
{
	return HasBehavior(BehaviorTag);
}

void USBCombatComponent::ServerRequestFire_Implementation(FGameplayTag BehaviorTag, int32 PredictionId)
{
	USBWeaponBehavior* TargetWeapon = FindAvailableWeaponByTag(BehaviorTag);
	if (!TargetWeapon)
	{
		ClientRollbackFire(BehaviorTag, PredictionId);
		return;
	}

	USBWeaponBehaviorDefinition* Def = TargetWeapon->GetDefinition();
	if (!Def)
	{
		ClientRollbackFire(BehaviorTag, PredictionId);
		return;
	}

	// 1. Valida Cooldown
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (float* LastTime = LastExecutionTimes.Find(BehaviorTag))
	{
		if (CurrentTime - *LastTime < Def->FireRate)
		{
			ClientRollbackFire(BehaviorTag, PredictionId);
			return;
		}
	}

	FSBGameplayContext GameplayCtx;
	FSBFrameworkContext FrameworkCtx;
	FSBBehaviorContext Context = BuildBehaviorContext(0.f, GameplayCtx, FrameworkCtx);

	// 2. Valida Tags e Recursos
	if (!TargetWeapon->CanEnter(Context))
	{
		ClientRollbackFire(BehaviorTag, PredictionId);
		return;
	}

	// 3. Executa o disparo oficial no servidor
	bool bSuccess = RequestWeaponBehavior(BehaviorTag, PredictionId);
	if (!bSuccess)
	{
		ClientRollbackFire(BehaviorTag, PredictionId);
	}
}

bool USBCombatComponent::ServerRequestFire_Validate(FGameplayTag BehaviorTag, int32 PredictionId)
{
	if (!CombatRPCLimiter.AllowRPC(GetWorld(), 20.0f))
	{
		return false;
	}

	if (!FindAvailableWeaponByTag(BehaviorTag))
	{
		return false;
	}

	return true;
}

void USBCombatComponent::ServerStopFire_Implementation(FGameplayTag BehaviorTag)
{
	StopWeaponBehavior(BehaviorTag, false);
}

bool USBCombatComponent::ServerStopFire_Validate(FGameplayTag BehaviorTag)
{
	if (!CombatRPCLimiter.AllowRPC(GetWorld(), 20.0f))
	{
		return false;
	}
	return true;
}

void USBCombatComponent::ClientRollbackFire_Implementation(FGameplayTag BehaviorTag, int32 PredictionId)
{
	// 1. Para o comportamento de arma localmente
	StopWeaponBehavior(BehaviorTag, true);

	// 2. Reverte os atributos locais correspondentes a essa predição
	if (CachedAttributeComponent)
	{
		FGameplayTag AmmoTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Weapon.Ammo"));
		CachedAttributeComponent->ClientRollbackPrediction(AmmoTag, PredictionId);

		FGameplayTag ManaTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Mana"));
		CachedAttributeComponent->ClientRollbackPrediction(ManaTag, PredictionId);
	}
}

void USBCombatComponent::OnBehaviorEjected(FGameplayTag BehaviorTag, bool bSkipServerNotify, bool bSkipClientNotify)
{
	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();
	const bool bIsLocallyControlled = GIsAutomationTesting || (Owner && Cast<APawn>(Owner) && Cast<APawn>(Owner)->IsLocallyControlled());

	// Notifica o servidor se for o cliente local
	if (!bIsServer && bIsLocallyControlled && !bSkipServerNotify)
	{
		ServerStopFire(BehaviorTag);
	}
}

USBWeaponBehavior* USBCombatComponent::FindAvailableWeaponByTag(FGameplayTag Tag) const
{
	return Cast<USBWeaponBehavior>(USBBehaviorStackComponent::FindAvailableBehaviorByTag(Tag));
}

USBWeaponBehavior* USBCombatComponent::FindActiveWeaponByTag(FGameplayTag Tag) const
{
	return Cast<USBWeaponBehavior>(USBBehaviorStackComponent::FindActiveBehaviorByTag(Tag));
}

void USBTestCombatComponent::ClientRollbackFire_Implementation(FGameplayTag BehaviorTag, int32 PredictionId)
{
	LastRollbackTag = BehaviorTag;
	LastRollbackPredictionId = PredictionId;
	Super::ClientRollbackFire_Implementation(BehaviorTag, PredictionId);
}

void USBCombatComponent::OnItemEquipped(FGameplayTag EventTag, UObject* Payload)
{
	if (!Payload) return;

	if (Payload->GetClass()->ImplementsInterface(USBEquipEventPayloadInterface::StaticClass()))
	{
		UObject* EquippableFragment = ISBEquipEventPayloadInterface::Execute_GetEquippableFragment(Payload);
		if (EquippableFragment && EquippableFragment->GetClass()->ImplementsInterface(USBEquippableInterface::StaticClass()))
		{
			UClass* BehaviorClass = ISBEquippableInterface::Execute_GetWeaponBehaviorClass(EquippableFragment);
			UPrimaryDataAsset* RawDef = ISBEquippableInterface::Execute_GetWeaponDefinitionAsset(EquippableFragment);
			USBWeaponBehaviorDefinition* DefAsset = Cast<USBWeaponBehaviorDefinition>(RawDef);

			if (BehaviorClass && DefAsset && BehaviorClass->IsChildOf(USBWeaponBehavior::StaticClass()))
			{
				FGameplayTag WeaponTag = DefAsset->BehaviorTag;
				if (!FindAvailableWeaponByTag(WeaponTag))
				{
					USBWeaponBehavior* NewBehavior = NewObject<USBWeaponBehavior>(this, BehaviorClass);
					NewBehavior->Initialize(this, DefAsset);
					AvailableBehaviors.Add(NewBehavior);
				}
			}
		}
	}
}

void USBCombatComponent::OnItemUnequipped(FGameplayTag EventTag, UObject* Payload)
{
	if (!Payload) return;

	if (Payload->GetClass()->ImplementsInterface(USBEquipEventPayloadInterface::StaticClass()))
	{
		UObject* EquippableFragment = ISBEquipEventPayloadInterface::Execute_GetEquippableFragment(Payload);
		if (EquippableFragment && EquippableFragment->GetClass()->ImplementsInterface(USBEquippableInterface::StaticClass()))
		{
			UPrimaryDataAsset* RawDef = ISBEquippableInterface::Execute_GetWeaponDefinitionAsset(EquippableFragment);
			USBWeaponBehaviorDefinition* DefAsset = Cast<USBWeaponBehaviorDefinition>(RawDef);

			if (DefAsset)
			{
				FGameplayTag WeaponTag = DefAsset->BehaviorTag;
				USBWeaponBehavior* Behavior = FindAvailableWeaponByTag(WeaponTag);
				if (Behavior)
				{
					StopWeaponBehavior(WeaponTag, false);
					AvailableBehaviors.Remove(Behavior);
				}
			}
		}
	}
}

void USBCombatComponent::GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const
{
	Super::GetDebugDescription_Implementation(OutDebugLines);

	FSBDebugLine Header;
	Header.Label = TEXT("Combat Component Telemetry");
	Header.bIsHeader = true;
	OutDebugLines.Add(Header);

	// Weapon Behaviors list
	FSBDebugLine WeaponListHeader;
	WeaponListHeader.Label = TEXT("Available Weapons");
	WeaponListHeader.bIsHeader = true;
	OutDebugLines.Add(WeaponListHeader);

	for (const auto& Behavior : AvailableBehaviors)
	{
		if (Behavior)
		{
			FSBDebugLine Line;
			Line.Label = Behavior->GetBehaviorTag().ToString();
			Line.Value = FString::Printf(TEXT("Class: %s"), *Behavior->GetClass()->GetName());
			OutDebugLines.Add(Line);
		}
	}
}
