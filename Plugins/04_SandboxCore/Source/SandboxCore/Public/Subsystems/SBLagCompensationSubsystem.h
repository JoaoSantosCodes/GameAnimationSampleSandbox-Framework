#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "GameFramework/Character.h"
#include "SBLagCompensationSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FSBSavedPosition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|LagCompensation")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|LagCompensation")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|LagCompensation")
	float Timestamp = 0.0f;
};

USTRUCT(BlueprintType)
struct FSBCharacterHistory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|LagCompensation")
	TArray<FSBSavedPosition> Positions;
};

/**
 * Subsistema autoritativo no Servidor para compensação de lag (Network Rewind / Backtracking).
 * Armazena o histórico de posições dos personagens para permitir traços de colisão compensados temporariamente no passado do cliente.
 */
UCLASS(BlueprintType)
class SANDBOXCORE_API USBLagCompensationSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// UWorldSubsystem interface
	virtual void PostInitialize() override;

	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual bool IsTickable() const override { return !IsTemplate(); }
	virtual TStatId GetStatId() const override;

	/**
	 * Grava as posições atuais de todos os personagens no histórico.
	 */
	void RecordPositions();

	/**
	 * Move temporariamente todos os personagens (exceto o atirador) para a posição em que estavam no tempo alvo.
	 * @param TargetTime - Marca de tempo no passado para interpolar as posições.
	 * @param OutOriginalTransforms - Mapa onde serão armazenadas as transformações originais para posterior restauração.
	 */
	void RewindPositions(float TargetTime, TMap<TWeakObjectPtr<ACharacter>, FTransform>& OutOriginalTransforms);

	/**
	 * Restaura as posições de todos os personagens para suas transformações originais do frame atual.
	 * @param OriginalTransforms - Mapa contendo as transformações originais retornadas por RewindPositions.
	 */
	void RestorePositions(const TMap<TWeakObjectPtr<ACharacter>, FTransform>& OriginalTransforms);

	/**
	 * Helper para expor o histórico para testes de unidade e depuração.
	 */
	const TMap<TWeakObjectPtr<ACharacter>, FSBCharacterHistory>& GetHistoryMap() const { return HistoryMap; }

protected:
	bool InterpolatePosition(const FSBCharacterHistory& History, float TargetTime, FVector& OutLocation, FRotator& OutRotation) const;

private:
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<ACharacter>, FSBCharacterHistory> HistoryMap;

	// Limite de 1.0 segundo de histórico para evitar consumo excessivo de memória e exploits de rewind infinito.
	const float MaxHistoryDuration = 1.0f;
};
