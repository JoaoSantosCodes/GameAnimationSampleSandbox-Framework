// Copyright 2026 João Santos. All Rights Reserved.
#include "AnimNotifies/SBAnimNotify_Footstep.h"
#include "DataAssets/SBSurfaceEffectsDataAsset.h"
#include "Subsystems/SBCosmeticSaturationSubsystem.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"
#include "SBGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

USBAnimNotify_Footstep::USBAnimNotify_Footstep()
{
	FootSocketName = TEXT("Foot_L");
}

void USBAnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	// 1. Determina a posição de início e fim do trace descendente
	FVector StartLoc = MeshComp->GetSocketLocation(FootSocketName);
	if (StartLoc.IsZero())
	{
		StartLoc = Owner->GetActorLocation();
	}

	FVector EndLoc = StartLoc - FVector(0.0f, 0.0f, 150.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	QueryParams.bReturnPhysicalMaterial = true; // Crucial para retornar o Material Físico!

	EPhysicalSurface SurfaceType = SurfaceType_Default;

	// 2. Executa line trace
	if (World->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, QueryParams))
	{
		UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.Get();
		if (PhysMat)
		{
			SurfaceType = PhysMat->SurfaceType;
		}
	}

	// 3. Obtém sons e efeitos mapeados no Data Asset
	USoundBase* SoundToPlay = nullptr;
	UObject* VisualEffectToPlay = nullptr;

	if (SurfaceEffectsConfig)
	{
		SurfaceEffectsConfig->GetEffectsForSurface(SurfaceType, SoundToPlay, VisualEffectToPlay);
	}

	// 4. Validação e Controle de Saturação de Cosméticos (Anti-Saturação)
	bool bAudioAllowed = true;
	USBCosmeticSaturationSubsystem* SaturationSubsystem = World->GetSubsystem<USBCosmeticSaturationSubsystem>();

	if (SoundToPlay && SaturationSubsystem)
	{
		// Permite reproduzir se respeitar o cooldown por célula espacial 3D
		bAudioAllowed = SaturationSubsystem->AllowSound(SoundToPlay, HitResult.ImpactPoint, 0.08f);
	}

	if (SoundToPlay && bAudioAllowed)
	{
		UGameplayStatics::PlaySoundAtLocation(World, SoundToPlay, HitResult.ImpactPoint);
	}

	// 5. Publicação do evento no Event Bus do Sandbox (Desacoplamento de UI/VFX)
	UGameInstance* GI = World->GetGameInstance();
	USBEventSubsystem* EventSubsystem = GI ? GI->GetSubsystem<USBEventSubsystem>() : nullptr;

	if (EventSubsystem)
	{
		USBFootstepEventPayload* Payload = NewObject<USBFootstepEventPayload>(EventSubsystem);
		Payload->TargetPawn = Cast<APawn>(Owner);
		Payload->Location = HitResult.ImpactPoint;
		Payload->SurfaceType = SurfaceType;
		Payload->FootSocketName = FootSocketName;

		EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Character_Footstep, Payload);
	}
}
