#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInterface.h"
#include "SBCombatFXTypes.generated.h"

UENUM(BlueprintType)
enum class ESBCombatFXType : uint8
{
	WeaponTrail UMETA(DisplayName = "WeaponTrail"),
	SocketEmitter UMETA(DisplayName = "SocketEmitter"),
	ImpactDecal UMETA(DisplayName = "ImpactDecal"),
	SurfaceSplash UMETA(DisplayName = "SurfaceSplash")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBWeaponTrailConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	FName StartSocketName = FName("Weapon_Base");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	FName EndSocketName = FName("Weapon_Tip");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	float Width = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBImpactDecalConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	TObjectPtr<UMaterialInterface> DecalMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	FVector DecalSize = FVector(20.0f, 20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	float LifeSpan = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	float FadeScreenSize = 0.01f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBCombatFXRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	ESBCombatFXType FXType = ESBCombatFXType::WeaponTrail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	TObjectPtr<USceneComponent> AttachToComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFX")
	FSBImpactDecalConfig DecalConfig;
};
