#include "Actors/SBPortalActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Subsystems/SBPortalSubsystem.h"
#include "Engine/World.h"

ASBPortalActor::ASBPortalActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(RootComponent);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

FVector ASBPortalActor::GetTeleportSpawnLocation() const
{
	return GetActorLocation() + GetActorForwardVector() * 150.0f;
}

void ASBPortalActor::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (USBPortalSubsystem* Subsystem = World->GetSubsystem<USBPortalSubsystem>())
		{
			Subsystem->RegisterPortal(this);
		}
	}

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ASBPortalActor::HandleBeginOverlap);
}

void ASBPortalActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TriggerBox->OnComponentBeginOverlap.RemoveDynamic(this, &ASBPortalActor::HandleBeginOverlap);

	if (UWorld* World = GetWorld())
	{
		if (USBPortalSubsystem* Subsystem = World->GetSubsystem<USBPortalSubsystem>())
		{
			Subsystem->UnregisterPortal(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ASBPortalActor::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bAutoTeleportOnOverlap && OtherActor && OtherActor != this)
	{
		if (UWorld* World = GetWorld())
		{
			if (USBPortalSubsystem* Subsystem = World->GetSubsystem<USBPortalSubsystem>())
			{
				Subsystem->RequestTeleport(OtherActor, Destination, PortalInfo.PortalTag);
			}
		}
	}
}
