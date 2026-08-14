#include "Components/SBCameraComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBMovementComponent.h"
#include "Camera/Modes/SBCameraMode.h"
#include "Camera/DataAssets/SBCameraModeDefinition.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"

USBCameraComponent::USBCameraComponent()
	: Super(FObjectInitializer::Get())
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	// Define o tick da câmera para rodar após a física e movimentação, evitando trepidação (jitter)
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void USBCameraComponent::OnInitialize_Implementation()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		CachedStateComponent = Owner->FindComponentByClass<USBStateComponent>();

		// Prerrequisito de tick: Garante que o USBMovementComponent processe as mutações da pilha física
		// antes do CameraComponent atualizar e interpolar no mesmo frame
		UActorComponent* MovementComp = Owner->FindComponentByClass<USBMovementComponent>();
		if (MovementComp)
		{
			AddTickPrerequisiteComponent(MovementComp);
		}
	}

	// Inscreve-se nos callbacks de alteração de tags de estado
	if (CachedStateComponent)
	{
		CachedStateComponent->OnStateChanged.AddDynamic(this, &USBCameraComponent::OnStateTagChanged);
	}
}

void USBCameraComponent::OnReady_Implementation()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		CachedSpringArmComponent = Owner->FindComponentByClass<USpringArmComponent>();
		CachedCameraComponent = Owner->FindComponentByClass<UCameraComponent>();
	}

	// Garante uma reconstrução inicial no início do jogo
	bStackChangePending = true;
}

void USBCameraComponent::OnShutdown_Implementation()
{
	if (CachedStateComponent)
	{
		CachedStateComponent->OnStateChanged.RemoveAll(this);
	}

	ActiveCameraModes.Empty();
	AvailableCameraModes.Empty();
}

void USBCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	if (GIsAutomationTesting)
	{
		if (bStackChangePending)
		{
			RebuildCameraStack();
		}

		FSBCameraContext Context;
		Context.Character = Cast<ACharacter>(Owner);
		Context.SpringArmComponent = CachedSpringArmComponent;
		Context.CameraComponent = CachedCameraComponent;
		Context.DeltaTime = DeltaTime;

		for (USBCameraMode* Mode : ActiveCameraModes)
		{
			if (Mode)
			{
				Mode->Update(DeltaTime, Context);
			}
		}

		if (ActiveCameraModes.Num() > 0 && ActiveCameraModes[0])
		{
			USBCameraModeDefinition* TopDef = ActiveCameraModes[0]->GetDefinition();
			if (TopDef)
			{
				float TargetFOV = TopDef->TargetFOV;
				float TargetArmLength = TopDef->TargetArmLength;
				FVector TargetOffset = TopDef->TargetSocketOffset;
				float BlendSpeed = TopDef->BlendSpeed;

				if (CachedCameraComponent)
				{
					CachedCameraComponent->FieldOfView = FMath::FInterpTo(
						CachedCameraComponent->FieldOfView,
						TargetFOV,
						DeltaTime,
						BlendSpeed
					);
				}

				if (CachedSpringArmComponent)
				{
					CachedSpringArmComponent->TargetArmLength = FMath::FInterpTo(
						CachedSpringArmComponent->TargetArmLength,
						TargetArmLength,
						DeltaTime,
						BlendSpeed
					);

					CachedSpringArmComponent->SocketOffset = FMath::VInterpTo(
						CachedSpringArmComponent->SocketOffset,
						TargetOffset,
						DeltaTime,
						BlendSpeed
					);
				}
			}
		}
		return;
	}

	// Otimização de CPU e Suporte a Spectator/Replay:
	// Só processa câmera se for controlado localmente OU se for o ViewTarget ativo do PlayerController local
	APlayerController* LocalPC = GEngine ? GEngine->GetFirstLocalPlayerController(GetWorld()) : nullptr;
	const bool bIsViewTarget = LocalPC && LocalPC->GetViewTarget() == Owner;
	const bool bIsLocallyControlled = PawnOwner->IsLocallyControlled();

	if (!bIsLocallyControlled && !bIsViewTarget)
	{
		return;
	}

	// 1. Coalesce: Reconstrói a pilha se houver mudanças agendadas neste tick
	if (bStackChangePending)
	{
		RebuildCameraStack();
	}

	// 2. Prepara o contexto de câmera do frame
	FSBCameraContext Context;
	Context.Character = Cast<ACharacter>(Owner);
	Context.SpringArmComponent = CachedSpringArmComponent;
	Context.CameraComponent = CachedCameraComponent;
	Context.DeltaTime = DeltaTime;

	// 3. Atualiza todos os modos ativos (permite que mantenham estados internos consistentemente)
	for (USBCameraMode* Mode : ActiveCameraModes)
	{
		if (Mode)
		{
			Mode->Update(DeltaTime, Context);
		}
	}

	// 4. Interpolador Suave (Blending): Aplica o modo do topo (maior prioridade)
	if (ActiveCameraModes.Num() > 0 && ActiveCameraModes[0])
	{
		USBCameraModeDefinition* TopDef = ActiveCameraModes[0]->GetDefinition();
		if (TopDef)
		{
			float TargetFOV = TopDef->TargetFOV;
			float TargetArmLength = TopDef->TargetArmLength;
			FVector TargetOffset = TopDef->TargetSocketOffset;
			float BlendSpeed = TopDef->BlendSpeed;

			// Interpola Field of View (FOV)
			if (CachedCameraComponent)
			{
				CachedCameraComponent->FieldOfView = FMath::FInterpTo(
					CachedCameraComponent->FieldOfView,
					TargetFOV,
					DeltaTime,
					BlendSpeed
				);
			}

			// Interpola Spring Arm Length e Socket Offset
			if (CachedSpringArmComponent)
			{
				CachedSpringArmComponent->TargetArmLength = FMath::FInterpTo(
					CachedSpringArmComponent->TargetArmLength,
					TargetArmLength,
					DeltaTime,
					BlendSpeed
				);

				CachedSpringArmComponent->SocketOffset = FMath::VInterpTo(
					CachedSpringArmComponent->SocketOffset,
					TargetOffset,
					DeltaTime,
					BlendSpeed
				);
			}
		}
	}
}

void USBCameraComponent::OnStateTagChanged(FGameplayTag Tag, bool bAdded)
{
	// Apenas agenda a alteração para processar no início do TickComponent do frame
	bStackChangePending = true;
}

void USBCameraComponent::RebuildCameraStack()
{
	bStackChangePending = false;

	if (!CachedStateComponent) return;

	TArray<USBCameraMode*> NewActiveModes;

	// 1. Coleta os modos ativados com base em suas tags no StateComponent
	for (USBCameraModeDefinition* Def : CameraModeConfigs)
	{
		if (Def && Def->ActivationTag.IsValid() && CachedStateComponent->HasTag(Def->ActivationTag))
		{
			USBCameraMode* ModeInstance = GetOrCreateCameraModeInstance(Def);
			if (ModeInstance)
			{
				NewActiveModes.Add(ModeInstance);
			}
		}
	}

	// 2. Ordena de forma decrescente baseada na prioridade (Maior prioridade assume o índice 0)
	NewActiveModes.StableSort([](const USBCameraMode& A, const USBCameraMode& B)
	{
		return A.GetPriority() > B.GetPriority();
	});

	// Prepara contexto para disparar transições
	FSBCameraContext Context;
	Context.Character = Cast<ACharacter>(GetOwner());
	Context.SpringArmComponent = CachedSpringArmComponent;
	Context.CameraComponent = CachedCameraComponent;
	Context.DeltaTime = 0.0f;

	// 3. Notifica a entrada dos novos modos adicionados
	for (USBCameraMode* Mode : NewActiveModes)
	{
		if (Mode && !ActiveCameraModes.Contains(Mode))
		{
			Mode->Enter(Context);
		}
	}

	// 4. Notifica a saída dos antigos modos removidos
	for (USBCameraMode* Mode : ActiveCameraModes)
	{
		if (Mode && !NewActiveModes.Contains(Mode))
		{
			Mode->Exit(Context);
		}
	}

	// Atualiza a pilha ativa
	ActiveCameraModes = NewActiveModes;
}

USBCameraMode* USBCameraComponent::GetOrCreateCameraModeInstance(USBCameraModeDefinition* Def)
{
	if (!Def) return nullptr;

	// Procura nas instâncias pré-alocadas
	for (USBCameraMode* Mode : AvailableCameraModes)
	{
		if (Mode && Mode->GetDefinition() == Def)
		{
			return Mode;
		}
	}

	// Se não existir, instancia uma nova
	TSubclassOf<USBCameraMode> ModeClass = Def->CameraModeClass;
	if (!ModeClass)
	{
		ModeClass = USBCameraMode::StaticClass(); // Fallback para a classe base
	}

	USBCameraMode* NewMode = NewObject<USBCameraMode>(this, ModeClass);
	if (NewMode)
	{
		NewMode->Initialize(this, Def);
		AvailableCameraModes.Add(NewMode);
	}

	return NewMode;
}

void USBMockCameraMode::Update_Implementation(float DeltaTime, const FSBCameraContext& Context)
{
	Super::Update_Implementation(DeltaTime, Context);
	UpdateCount++;
}
