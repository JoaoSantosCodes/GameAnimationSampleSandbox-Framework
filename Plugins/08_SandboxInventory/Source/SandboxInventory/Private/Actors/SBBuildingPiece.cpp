#include "Actors/SBBuildingPiece.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

ASBBuildingPiece::ASBBuildingPiece()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
	CollisionBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MaxHealth = 100.0f;
	Health = MaxHealth;
}

void ASBBuildingPiece::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		Health = MaxHealth;
	}
}

void ASBBuildingPiece::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASBBuildingPiece, Health);
	DOREPLIFETIME(ASBBuildingPiece, OwnerPlayerName);
}

void ASBBuildingPiece::SetPreviewMode(bool bIsPreview)
{
	if (bIsPreview)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	else
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
	}
}

void ASBBuildingPiece::ServerTakeDamage(float DamageAmount)
{
	if (!HasAuthority()) return;

	Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
	if (Health <= 0.0f)
	{
		Destroy();
	}
}

void ASBBuildingPiece::OnRep_Health()
{
	// Can be used for visual effects or HUD updates on clients
}
