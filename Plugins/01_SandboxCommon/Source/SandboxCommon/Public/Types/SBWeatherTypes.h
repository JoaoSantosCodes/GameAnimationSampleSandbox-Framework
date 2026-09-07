// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBWeatherTypes.generated.h"

/**
 * Enum de tipos de clima suportados
 */
UENUM(BlueprintType)
enum class ESBWeatherType : uint8
{
	Clear UMETA(DisplayName = "Clear"),
	Cloudy UMETA(DisplayName = "Cloudy"),
	Rain UMETA(DisplayName = "Rain"),
	Thunderstorm UMETA(DisplayName = "Thunderstorm"),
	Snow UMETA(DisplayName = "Snow"),
	Fog UMETA(DisplayName = "Fog"),
	Sandstorm UMETA(DisplayName = "Sandstorm")
};

/**
 * Estado atual e parâmetros ambientais do clima
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBWeatherState
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Weather")
	ESBWeatherType CurrentWeather = ESBWeatherType::Clear;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Weather")
	ESBWeatherType TargetWeather = ESBWeatherType::Clear;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float WeatherIntensity = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float AmbientTemperature = 22.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float WindSpeed = 10.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FGameplayTag WeatherTag;
};

/**
 * Representação de tempo in-game (dia, hora, minuto e período)
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBTimeOfDay
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time")
	int32 Day = 1;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time")
	int32 Hour = 8;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time")
	int32 Minute = 0;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time")
	float TimeOfDayNormalized = 0.333f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time")
	bool bIsDaytime = true;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time")
	bool bIsNighttime = false;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Time")
	FGameplayTag TimePeriodTag;
};
