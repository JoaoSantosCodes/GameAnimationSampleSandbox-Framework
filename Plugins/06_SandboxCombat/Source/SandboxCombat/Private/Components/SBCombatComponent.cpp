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
#include "Net/UnrealNetwork.h"

USBCombatComponent::USBCombatComponent()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USBCombatComponent, SpawnedWeapons);
}

void USBCombatComponent::SetWeaponVisualActive(FGameplayTag WeaponTag, bool bActive)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	AActor* WeaponActor = GetSpawnedWeaponActor(WeaponTag);
	if (!WeaponActor)
	{
		return;
	}

	ACharacter* CharOwner = Cast<ACharacter>(GetOwner());
	if (!CharOwner || !CharOwner->GetMesh())
	{
		return;
	}

	USBWeaponBehavior* WeaponBehavior = FindAvailableWeaponByTag(WeaponTag);
	if (!WeaponBehavior)
	{
		return;
	}

	USBWeaponBehaviorDefinition* Def = WeaponBehavior->GetDefinition();
	if (!Def)
	{
		return;
	}

	FName TargetSocket = bActive ? Def->ActiveSocketName : Def->HolsterSocketName;

	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
	WeaponActor->AttachToComponent(CharOwner->GetMesh(), AttachRules, TargetSocket);
}

AActor* USBCombatComponent::GetSpawnedWeaponActor(FGameplayTag WeaponTag) const
{
	for (const FSBSpawnedWeaponEntry& Entry : SpawnedWeapons)
	{
		if (Entry.WeaponTag == WeaponTag)
		{
			return Entry.WeaponActor;
		}
	}
	return nullptr;
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

void USBCombatComponent::OnReady_Implementation()
{
	Super::OnReady_Implementation();

	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		USBAttributeComponent* AttrComp = Owner->FindComponentByClass<USBAttributeComponent>();
		if (AttrComp)
		{
			FGameplayTag AmmoTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Weapon.Ammo"), false);
			FSBAttribute DummyAmmo;
			if (AmmoTag.IsValid() && !AttrComp->GetAttribute(AmmoTag, DummyAmmo))
			{
				FSBAttribute AmmoAttr;
				AmmoAttr.BaseValue = 30.f;
				AmmoAttr.CurrentValue = 30.f;
				AmmoAttr.MaxValue = 30.f;
				AmmoAttr.MinValue = 0.f;
				AmmoAttr.bIsPrivate = true; // Replicada via COND_OwnerOnly
				AttrComp->RegisterAttribute(AmmoTag, AmmoAttr);
			}
		}
	}
}

void USBCombatComponent::OnShutdown_Implementation()
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		for (FSBSpawnedWeaponEntry& Entry : SpawnedWeapons)
		{
			if (Entry.WeaponActor)
			{
				Entry.WeaponActor->Destroy();
			}
		}
		SpawnedWeapons.Empty();
	}

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

	// Destrói atores visuais anteriores
	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();
	if (bIsServer)
	{
		for (FSBSpawnedWeaponEntry& Entry : SpawnedWeapons)
		{
			if (Entry.WeaponActor)
			{
				Entry.WeaponActor->Destroy();
			}
		}
		SpawnedWeapons.Empty();
	}

	// Instancia os behaviors configurados e spawna atores visuais
	for (const FSBWeaponConfigEntry& Entry : NewConfig->ConfiguredWeapons)
	{
		if (Entry.BehaviorClass && Entry.DefinitionAsset)
		{
			USBWeaponBehavior* NewBehavior = NewObject<USBWeaponBehavior>(this, Entry.BehaviorClass);
			NewBehavior->Initialize(this, Entry.DefinitionAsset);
			AvailableBehaviors.Add(NewBehavior);

			if (bIsServer && Entry.DefinitionAsset->WeaponActorClass)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = Owner;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				
				AActor* NewWeaponActor = GetWorld()->SpawnActor<AActor>(Entry.DefinitionAsset->WeaponActorClass, Owner->GetActorLocation(), Owner->GetActorRotation(), SpawnParams);
				if (NewWeaponActor)
				{
					if (NewWeaponActor->GetRootComponent())
					{
						NewWeaponActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
					}

					FSBSpawnedWeaponEntry NewEntry;
					NewEntry.WeaponTag = Entry.DefinitionAsset->BehaviorTag;
					NewEntry.WeaponActor = NewWeaponActor;
					SpawnedWeapons.Add(NewEntry);

					ACharacter* CharOwner = Cast<ACharacter>(Owner);
					if (CharOwner && CharOwner->GetMesh())
					{
						FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
						NewWeaponActor->AttachToComponent(CharOwner->GetMesh(), AttachRules, Entry.DefinitionAsset->HolsterSocketName);
					}
				}
			}
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

					// Spawn do Actor Visual no Servidor
					AActor* Owner = GetOwner();
					if (Owner && Owner->HasAuthority() && DefAsset->WeaponActorClass)
					{
						FActorSpawnParameters SpawnParams;
						SpawnParams.Owner = Owner;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						
						AActor* NewWeaponActor = GetWorld()->SpawnActor<AActor>(DefAsset->WeaponActorClass, Owner->GetActorLocation(), Owner->GetActorRotation(), SpawnParams);
						if (NewWeaponActor)
						{
							NewWeaponActor->SetReplicates(true);
							if (NewWeaponActor->GetRootComponent())
							{
								NewWeaponActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
							}

							FSBSpawnedWeaponEntry NewEntry;
							NewEntry.WeaponTag = WeaponTag;
							NewEntry.WeaponActor = NewWeaponActor;
							SpawnedWeapons.Add(NewEntry);

							// Anexa inicialmente no holster
							ACharacter* CharOwner = Cast<ACharacter>(Owner);
							if (CharOwner && CharOwner->GetMesh())
							{
								FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
								NewWeaponActor->AttachToComponent(CharOwner->GetMesh(), AttachRules, DefAsset->HolsterSocketName);
							}
						}
					}
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

					// Destruição do Actor Visual no Servidor
					AActor* Owner = GetOwner();
					if (Owner && Owner->HasAuthority())
					{
						int32 FoundIndex = INDEX_NONE;
						for (int32 i = 0; i < SpawnedWeapons.Num(); ++i)
						{
							if (SpawnedWeapons[i].WeaponTag == WeaponTag)
							{
								FoundIndex = i;
								break;
							}
						}

						if (FoundIndex != INDEX_NONE)
						{
							AActor* WeaponActor = SpawnedWeapons[FoundIndex].WeaponActor;
							if (WeaponActor)
							{
								WeaponActor->Destroy();
							}
							SpawnedWeapons.RemoveAt(FoundIndex);
						}
					}
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

void USBCombatComponent::AddAgro(APawn* TargetPawn, float Amount)
{
	if (!TargetPawn || Amount <= 0.0f)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		float& CurrentAgro = AgroTable.FindOrAdd(TargetPawn);
		CurrentAgro += Amount;
	}
}

void USBCombatComponent::ClearAgro(APawn* TargetPawn)
{
	if (!TargetPawn)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		AgroTable.Remove(TargetPawn);
	}
}

void USBCombatComponent::ClearAllAgro()
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		AgroTable.Empty();
	}
}

APawn* USBCombatComponent::GetHighestAgroTarget() const
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return nullptr;
	}

	APawn* BestTarget = nullptr;
	float HighestAgro = -1.0f;

	TArray<TObjectPtr<APawn>> InvalidKeys;

	for (auto It = AgroTable.CreateConstIterator(); It; ++It)
	{
		APawn* PawnKey = It.Key().Get();
		if (!IsValid(PawnKey))
		{
			InvalidKeys.Add(It.Key());
			continue;
		}

		if (It.Value() > HighestAgro)
		{
			HighestAgro = It.Value();
			BestTarget = PawnKey;
		}
	}

	if (InvalidKeys.Num() > 0)
	{
		USBCombatComponent* MutableThis = const_cast<USBCombatComponent*>(this);
		for (const auto& Key : InvalidKeys)
		{
			MutableThis->AgroTable.Remove(Key);
		}
	}

	return BestTarget;
}
