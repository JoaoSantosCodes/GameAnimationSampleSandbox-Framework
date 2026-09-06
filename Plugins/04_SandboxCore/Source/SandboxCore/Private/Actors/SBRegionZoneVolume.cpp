#include "Actors/SBRegionZoneVolume.h"
#include "Components/BoxComponent.h"
#include "Subsystems/SBRegionSubsystem.h"
#include "Engine/World.h"

ASBRegionZoneVolume::ASBRegionZoneVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
}

void ASBRegionZoneVolume::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ASBRegionZoneVolume::HandleBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ASBRegionZoneVolume::HandleEndOverlap);
}

void ASBRegionZoneVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TriggerBox->OnComponentBeginOverlap.RemoveDynamic(this, &ASBRegionZoneVolume::HandleBeginOverlap);
	TriggerBox->OnComponentEndOverlap.RemoveDynamic(this, &ASBRegionZoneVolume::HandleEndOverlap);

	Super::EndPlay(EndPlayReason);
}

void ASBRegionZoneVolume::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		if (UWorld* World = GetWorld())
		{
			if (USBRegionSubsystem* Subsystem = World->GetSubsystem<USBRegionSubsystem>())
			{
				Subsystem->NotifyActorEnteredRegion(OtherActor, RegionData);
			}
		}
	}
}

void ASBRegionZoneVolume::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this)
	{
		if (UWorld* World = GetWorld())
		{
			if (USBRegionSubsystem* Subsystem = World->GetSubsystem<USBRegionSubsystem>())
			{
				Subsystem->NotifyActorExitedRegion(OtherActor, RegionData);
			}
		}
	}
}
