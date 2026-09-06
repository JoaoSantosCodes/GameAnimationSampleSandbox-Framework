#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Interfaces/SBSaveInterface.h"
#include "Types/SBWeatherTypes.h"
#include "SBWeatherSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnWeatherChanged, const FSBWeatherState&, NewWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnHourChanged, int32, NewHour);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnDayChanged, int32, NewDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnTimePeriodChanged, FGameplayTag, NewPeriodTag);

/**
 * Subsistema de gerenciamento de tempo contínuo (Ciclo Dia/Noite, Horas, Dias) e Clima Dinâmico
 */
UCLASS(BlueprintType)
class SANDBOXCORE_API USBWeatherSubsystem : public UTickableWorldSubsystem, public ISBSaveInterface
{
	GENERATED_BODY()

public:
	USBWeatherSubsystem();

	// UTickableWorldSubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return true; }

	// ISBSaveInterface implementation
	virtual bool SaveComponentData_Implementation(UObject* SavePayload) override;
	virtual bool LoadComponentData_Implementation(UObject* SavePayload) override;
	virtual int32 GetSavePriority_Implementation() const override { return 110; }

	// Configurações de tempo
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Time")
	void SetTimeOfDay(int32 NewHour, int32 NewMinute);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Time")
	void SetDay(int32 NewDay);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Time")
	void SetTimeScale(float NewTimeScale);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Time")
	void ForceAdvanceGameMinutes(float Minutes);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Time")
	FSBTimeOfDay GetTimeOfDay() const { return TimeOfDay; }

	// Configurações de clima
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Weather")
	void SetWeather(ESBWeatherType NewWeather, float InTransitionDuration = 5.0f);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Weather")
	FSBWeatherState GetWeatherState() const { return WeatherState; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Weather")
	FSBOnWeatherChanged OnWeatherChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Time")
	FSBOnHourChanged OnHourChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Time")
	FSBOnDayChanged OnDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Time")
	FSBOnTimePeriodChanged OnTimePeriodChanged;

private:
	void UpdateTimeOfDayState();
	void UpdateWeatherTransition(float DeltaTime);
	void ApplyDefaultWeatherParams(ESBWeatherType Weather, float& OutTemp, float& OutWind, FGameplayTag& OutTag);

	UPROPERTY(SaveGame)
	FSBTimeOfDay TimeOfDay;

	UPROPERTY(SaveGame)
	FSBWeatherState WeatherState;

	UPROPERTY(EditDefaultsOnly, Category = "Sandbox|Time")
	float TimeScale = 60.0f; // 1s real = 1 minuto in-game

	float MinuteAccumulator = 0.0f;
	float TransitionDuration = 0.0f;
	float TransitionElapsed = 0.0f;
	bool bIsTransitioning = false;

	float StartTemp = 22.0f;
	float TargetTemp = 22.0f;
	float StartWind = 10.0f;
	float TargetWind = 10.0f;
};
