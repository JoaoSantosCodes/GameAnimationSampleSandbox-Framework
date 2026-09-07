// Copyright 2026 João Santos. All Rights Reserved.
#include "Actors/SBAmbientZoneTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Sound/SoundBase.h"
#include "Subsystems/SBEventSubsystem.h"
#include "SBGameplayTags.h"

ASBAmbientZoneTrigger::ASBAmbientZoneTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->InitBoxExtent(FVector(250.0f, 250.0f, 250.0f));
	RootComponent = TriggerBox;

	FadeInDuration = 1.5f;
	FadeOutDuration = 1.5f;
	VolumeMultiplier = 1.0f;
}

void ASBAmbientZoneTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ASBAmbientZoneTrigger::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ASBAmbientZoneTrigger::OnOverlapEnd);
}

void ASBAmbientZoneTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* OverlappingPawn = Cast<APawn>(OtherActor);
	if (OverlappingPawn && (OverlappingPawn->IsLocallyControlled() || GIsAutomationTesting))
	{
		// 1. Processamento de Descoberta de Área
		if (bEnableAreaDiscovery)
		{
			USBEventSubsystem* EventSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBEventSubsystem>() : nullptr;
			if (EventSubsystem)
			{
				bool bIsFirstTime = !bHasBeenDiscovered;
				bHasBeenDiscovered = true;

				USBAreaDiscoveryPayload* Payload = NewObject<USBAreaDiscoveryPayload>(this);
				Payload->AreaName = AreaName;
				Payload->AreaDescription = AreaDescription;
				Payload->PresentationSequence = PresentationSequence;
				Payload->bIsFirstDiscovery = bIsFirstTime;

				FGameplayTag DiscoveryTag = FSBGameplayTags::Get().Event_Area_Discovered;
				if (DiscoveryTag.IsValid())
				{
					EventSubsystem->PublishEvent(DiscoveryTag, Payload);
				}
			}
		}

		// 2. Processamento do Som Ambiente
		if (AmbientSound)
		{
			UE_LOG(LogTemp, Log, TEXT("ASBAmbientZoneTrigger::OnOverlapBegin: Entering zone with sound %s"), *AmbientSound->GetName());
			
			if (!ActiveAudioComponent)
			{
				ActiveAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), AmbientSound, 0.0f, 1.0f, 0.0f, nullptr, true, false);
			}

			if (ActiveAudioComponent)
			{
				ActiveAudioComponent->FadeIn(FadeInDuration, VolumeMultiplier);
			}
		}
	}
}

void ASBAmbientZoneTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!ActiveAudioComponent) return;

	APawn* OverlappingPawn = Cast<APawn>(OtherActor);
	if (OverlappingPawn && (OverlappingPawn->IsLocallyControlled() || GIsAutomationTesting))
	{
		UE_LOG(LogTemp, Log, TEXT("ASBAmbientZoneTrigger::OnOverlapEnd: Leaving zone"));
		ActiveAudioComponent->FadeOut(FadeOutDuration, 0.0f);
		ActiveAudioComponent = nullptr;
	}
}
