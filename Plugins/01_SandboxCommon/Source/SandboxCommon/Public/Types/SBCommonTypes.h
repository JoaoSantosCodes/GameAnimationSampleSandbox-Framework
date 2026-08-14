#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBCommonTypes.generated.h"

class APawn;
class ACharacter;
class AController;
class APlayerState;
class UWorld;
class UGameInstance;

UENUM(BlueprintType)
enum class ESBAttributeModifierType : uint8
{
	Override,
	Additive,
	Multiplicative
};

UENUM(BlueprintType)
enum class ESBEventPriority : uint8
{
	High = 0,
	Medium = 10,
	Low = 20,
	Lowest = 30
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBAttribute
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Attribute")
	float BaseValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Attribute")
	float CurrentValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Attribute")
	float MaxValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Attribute")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Attribute")
	float RegenRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Attribute")
	float RegenDelay = 0.0f;

	// Auxiliar to track last time modified (for delay/regen)
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Attribute")
	float LastModifiedTime = 0.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBAttributeModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	FGameplayTag SourceTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	ESBAttributeModifierType ModifierType = ESBAttributeModifierType::Additive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	float Magnitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	float Duration = 0.0f; // <= 0 for infinite/instant

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	int32 StackCount = 1;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<ACharacter> Character = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<AController> Controller = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	float DeltaTime = 0.0f;
};

UCLASS(BlueprintType)
class SANDBOXCOMMON_API USBBehaviorRegistry : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Registry")
	virtual UObject* GetOrInstantiateBehavior(FGameplayTag BehaviorTag, TSubclassOf<UObject> BehaviorClass);

protected:
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UObject>> InstantiatedBehaviors;
};

UCLASS(Abstract, BlueprintType)
class SANDBOXCOMMON_API USBModifierAggregator : public UObject
{
	GENERATED_BODY()
};

UENUM(BlueprintType)
enum class ESBModifierOperation : uint8
{
	Additive,
	Multiply,
	Override
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBModifierEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	FGameplayTag TargetStatTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	FGameplayTag SourceTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	ESBModifierOperation Operation = ESBModifierOperation::Additive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBGameplayContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	float DeltaSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<APawn> Pawn = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<ACharacter> Character = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<AController> Controller = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<APlayerState> PlayerState = nullptr;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBFrameworkContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<UWorld> World = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<UGameInstance> GameInstance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<UObject> EventSubsystem = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<UObject> AssetManager = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<UObject> SaveSubsystem = nullptr;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBBehaviorContext
{
	GENERATED_BODY()

	const FSBGameplayContext* GameplayContext = nullptr;

	const FSBFrameworkContext* FrameworkContext = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Context")
	TObjectPtr<UObject> FeatureContext = nullptr;
};
