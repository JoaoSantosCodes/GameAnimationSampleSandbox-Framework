#include "Subsystems/SBLagCompensationSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Stats/Stats.h"

void USBLagCompensationSubsystem::PostInitialize()
{
	Super::PostInitialize();
}

void USBLagCompensationSubsystem::Tick(float DeltaTime)
{
	RecordPositions();
}

TStatId USBLagCompensationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USBLagCompensationSubsystem, STATGROUP_Tickables);
}

void USBLagCompensationSubsystem::RecordPositions()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float CurrentTime = World->GetTimeSeconds();

	// 1. Limpa entradas de personagens que foram destruídos do mapa de histórico
	TArray<TWeakObjectPtr<ACharacter>> DeadCharacters;
	for (const auto& Pair : HistoryMap)
	{
		if (!Pair.Key.IsValid())
		{
			DeadCharacters.Add(Pair.Key);
		}
	}
	for (const auto& Key : DeadCharacters)
	{
		HistoryMap.Remove(Key);
	}

	// 2. Grava as posições atuais de todos os personagens válidos no mundo
	for (TActorIterator<ACharacter> It(World); It; ++It)
	{
		ACharacter* Char = *It;
		if (!Char || !IsValid(Char))
		{
			continue;
		}

		FSBCharacterHistory& History = HistoryMap.FindOrAdd(Char);

		FSBSavedPosition NewPos;
		NewPos.Location = Char->GetActorLocation();
		NewPos.Rotation = Char->GetActorRotation();
		NewPos.Timestamp = CurrentTime;

		History.Positions.Add(NewPos);

		// 3. Remove registros mais velhos que MaxHistoryDuration
		while (History.Positions.Num() > 0 && (CurrentTime - History.Positions[0].Timestamp > MaxHistoryDuration))
		{
			History.Positions.RemoveAt(0);
		}
	}
}

void USBLagCompensationSubsystem::RewindPositions(float TargetTime, TMap<TWeakObjectPtr<ACharacter>, FTransform>& OutOriginalTransforms)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	OutOriginalTransforms.Empty();

	// Iteramos por todos os personagens registrados no histórico
	for (auto& Pair : HistoryMap)
	{
		ACharacter* Char = Pair.Key.Get();
		if (!Char || !IsValid(Char))
		{
			continue;
		}

		// Armazena a transformação atual de autoridade antes de rebobinar
		OutOriginalTransforms.Add(Char, Char->GetActorTransform());

		FVector RewoundLocation;
		FRotator RewoundRotation;
		if (InterpolatePosition(Pair.Value, TargetTime, RewoundLocation, RewoundRotation))
		{
			// Teleporta fisicamente o colisor no servidor para a posição do passado
			Char->SetActorLocationAndRotation(RewoundLocation, RewoundRotation, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

void USBLagCompensationSubsystem::RestorePositions(const TMap<TWeakObjectPtr<ACharacter>, FTransform>& OriginalTransforms)
{
	for (const auto& Pair : OriginalTransforms)
	{
		ACharacter* Char = Pair.Key.Get();
		if (Char && IsValid(Char))
		{
			// Restaura a colisão e posição para o presente autoritativo no servidor
			Char->SetActorLocationAndRotation(Pair.Value.GetLocation(), Pair.Value.GetRotation().Rotator(), false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

bool USBLagCompensationSubsystem::InterpolatePosition(const FSBCharacterHistory& History, float TargetTime, FVector& OutLocation, FRotator& OutRotation) const
{
	if (History.Positions.Num() == 0)
	{
		return false;
	}

	// Se o tempo estiver fora dos limites do histórico, grampeia no limite correspondente
	if (TargetTime <= History.Positions[0].Timestamp)
	{
		OutLocation = History.Positions[0].Location;
		OutRotation = History.Positions[0].Rotation;
		return true;
	}

	if (TargetTime >= History.Positions.Last().Timestamp)
	{
		OutLocation = History.Positions.Last().Location;
		OutRotation = History.Positions.Last().Rotation;
		return true;
	}

	// Busca os dois pontos mais próximos contornando o tempo alvo
	int32 LeftIndex = 0;
	int32 RightIndex = 0;
	bool bFound = false;

	for (int32 i = 0; i < History.Positions.Num() - 1; ++i)
	{
		if (History.Positions[i].Timestamp <= TargetTime && TargetTime <= History.Positions[i + 1].Timestamp)
		{
			LeftIndex = i;
			RightIndex = i + 1;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		// Fallback para o último registro caso ocorra alguma inconsistência temporal
		OutLocation = History.Positions.Last().Location;
		OutRotation = History.Positions.Last().Rotation;
		return true;
	}

	const FSBSavedPosition& Left = History.Positions[LeftIndex];
	const FSBSavedPosition& Right = History.Positions[RightIndex];

	// Interpolação Linear (LERP de localização, SLERP esférico de rotação via Quats)
	float Alpha = (TargetTime - Left.Timestamp) / (Right.Timestamp - Left.Timestamp);
	OutLocation = FMath::Lerp(Left.Location, Right.Location, Alpha);

	FQuat LeftQuat = Left.Rotation.Quaternion();
	FQuat RightQuat = Right.Rotation.Quaternion();
	OutRotation = FQuat::Slerp(LeftQuat, RightQuat, Alpha).Rotator();

	return true;
}
