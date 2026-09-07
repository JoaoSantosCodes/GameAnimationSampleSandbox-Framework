// Copyright 2026 João Santos. All Rights Reserved.
#include "Subsystems/SBWeatherSubsystem.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "SBGameplayTags.h"

USBWeatherSubsystem::USBWeatherSubsystem()
{
}

void USBWeatherSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TimeOfDay.Day = 1;
	TimeOfDay.Hour = 8;
	TimeOfDay.Minute = 0;
	UpdateTimeOfDayState();

	WeatherState.CurrentWeather = ESBWeatherType::Clear;
	WeatherState.TargetWeather = ESBWeatherType::Clear;
	WeatherState.WeatherIntensity = 0.0f;
	WeatherState.AmbientTemperature = 22.0f;
	WeatherState.WindSpeed = 10.0f;
	WeatherState.WeatherTag = FSBGameplayTags::Get().State_Weather_Clear;

	bIsTransitioning = false;
}

void USBWeatherSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USBWeatherSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. Avanço de tempo
	if (TimeScale > 0.0f)
	{
		MinuteAccumulator += DeltaTime * TimeScale;
		if (MinuteAccumulator >= 60.0f)
		{
			float MinutesToAdd = FMath::FloorToFloat(MinuteAccumulator / 60.0f);
			MinuteAccumulator -= (MinutesToAdd * 60.0f);
			ForceAdvanceGameMinutes(MinutesToAdd);
		}
	}

	// 2. Transição de clima
	if (bIsTransitioning)
	{
		UpdateWeatherTransition(DeltaTime);
	}
}

TStatId USBWeatherSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USBWeatherSubsystem, STATGROUP_Tickables);
}

void USBWeatherSubsystem::SetTimeOfDay(int32 NewHour, int32 NewMinute)
{
	TimeOfDay.Hour = FMath::Clamp(NewHour, 0, 23);
	TimeOfDay.Minute = FMath::Clamp(NewMinute, 0, 59);
	UpdateTimeOfDayState();
	OnHourChanged.Broadcast(TimeOfDay.Hour);
}

void USBWeatherSubsystem::SetDay(int32 NewDay)
{
	TimeOfDay.Day = FMath::Max(1, NewDay);
	OnDayChanged.Broadcast(TimeOfDay.Day);
}

void USBWeatherSubsystem::SetTimeScale(float NewTimeScale)
{
	TimeScale = FMath::Max(0.0f, NewTimeScale);
}

void USBWeatherSubsystem::ForceAdvanceGameMinutes(float Minutes)
{
	if (Minutes <= 0.0f) return;

	int32 TotalMinutes = TimeOfDay.Minute + static_cast<int32>(Minutes);
	int32 HoursToAdd = TotalMinutes / 60;
	TimeOfDay.Minute = TotalMinutes % 60;

	if (HoursToAdd > 0)
	{
		int32 OldHour = TimeOfDay.Hour;
		int32 TotalHours = TimeOfDay.Hour + HoursToAdd;
		int32 DaysToAdd = TotalHours / 24;
		TimeOfDay.Hour = TotalHours % 24;

		if (DaysToAdd > 0)
		{
			TimeOfDay.Day += DaysToAdd;
			OnDayChanged.Broadcast(TimeOfDay.Day);
		}

		if (TimeOfDay.Hour != OldHour)
		{
			OnHourChanged.Broadcast(TimeOfDay.Hour);
		}
	}

	UpdateTimeOfDayState();
}

void USBWeatherSubsystem::SetWeather(ESBWeatherType NewWeather, float InTransitionDuration)
{
	WeatherState.TargetWeather = NewWeather;

	if (InTransitionDuration <= 0.0f)
	{
		WeatherState.CurrentWeather = NewWeather;
		ApplyDefaultWeatherParams(NewWeather, WeatherState.AmbientTemperature, WeatherState.WindSpeed, WeatherState.WeatherTag);
		bIsTransitioning = false;
		OnWeatherChanged.Broadcast(WeatherState);
		return;
	}

	StartTemp = WeatherState.AmbientTemperature;
	StartWind = WeatherState.WindSpeed;

	FGameplayTag DummyTag;
	ApplyDefaultWeatherParams(NewWeather, TargetTemp, TargetWind, DummyTag);

	TransitionDuration = InTransitionDuration;
	TransitionElapsed = 0.0f;
	bIsTransitioning = true;
}

void USBWeatherSubsystem::UpdateTimeOfDayState()
{
	TimeOfDay.TimeOfDayNormalized = (TimeOfDay.Hour * 60.0f + TimeOfDay.Minute) / 1440.0f;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	FGameplayTag OldPeriod = TimeOfDay.TimePeriodTag;

	if (TimeOfDay.Hour >= 5 && TimeOfDay.Hour < 7)
	{
		TimeOfDay.TimePeriodTag = Tags.State_Time_Dawn;
		TimeOfDay.bIsDaytime = true;
		TimeOfDay.bIsNighttime = false;
	}
	else if (TimeOfDay.Hour >= 7 && TimeOfDay.Hour < 18)
	{
		TimeOfDay.TimePeriodTag = Tags.State_Time_Day;
		TimeOfDay.bIsDaytime = true;
		TimeOfDay.bIsNighttime = false;
	}
	else if (TimeOfDay.Hour >= 18 && TimeOfDay.Hour < 20)
	{
		TimeOfDay.TimePeriodTag = Tags.State_Time_Dusk;
		TimeOfDay.bIsDaytime = false;
		TimeOfDay.bIsNighttime = true;
	}
	else
	{
		TimeOfDay.TimePeriodTag = Tags.State_Time_Night;
		TimeOfDay.bIsDaytime = false;
		TimeOfDay.bIsNighttime = true;
	}

	if (TimeOfDay.TimePeriodTag != OldPeriod)
	{
		OnTimePeriodChanged.Broadcast(TimeOfDay.TimePeriodTag);
	}
}

void USBWeatherSubsystem::UpdateWeatherTransition(float DeltaTime)
{
	TransitionElapsed += DeltaTime;
	float Alpha = FMath::Clamp(TransitionElapsed / TransitionDuration, 0.0f, 1.0f);

	WeatherState.AmbientTemperature = FMath::Lerp(StartTemp, TargetTemp, Alpha);
	WeatherState.WindSpeed = FMath::Lerp(StartWind, TargetWind, Alpha);
	WeatherState.WeatherIntensity = Alpha;

	if (Alpha >= 1.0f)
	{
		WeatherState.CurrentWeather = WeatherState.TargetWeather;
		ApplyDefaultWeatherParams(WeatherState.CurrentWeather, WeatherState.AmbientTemperature, WeatherState.WindSpeed, WeatherState.WeatherTag);
		bIsTransitioning = false;
		OnWeatherChanged.Broadcast(WeatherState);
	}
}

void USBWeatherSubsystem::ApplyDefaultWeatherParams(ESBWeatherType Weather, float& OutTemp, float& OutWind, FGameplayTag& OutTag)
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	switch (Weather)
	{
	case ESBWeatherType::Clear:
		OutTemp = 24.0f;
		OutWind = 8.0f;
		OutTag = Tags.State_Weather_Clear;
		break;
	case ESBWeatherType::Cloudy:
		OutTemp = 20.0f;
		OutWind = 15.0f;
		OutTag = Tags.State_Weather_Clear;
		break;
	case ESBWeatherType::Rain:
		OutTemp = 16.0f;
		OutWind = 25.0f;
		OutTag = Tags.State_Weather_Rain;
		break;
	case ESBWeatherType::Thunderstorm:
		OutTemp = 14.0f;
		OutWind = 50.0f;
		OutTag = Tags.State_Weather_Storm;
		break;
	case ESBWeatherType::Snow:
		OutTemp = -6.0f;
		OutWind = 18.0f;
		OutTag = Tags.State_Weather_Snow;
		break;
	case ESBWeatherType::Fog:
		OutTemp = 12.0f;
		OutWind = 4.0f;
		OutTag = Tags.State_Weather_Fog;
		break;
	case ESBWeatherType::Sandstorm:
		OutTemp = 39.0f;
		OutWind = 65.0f;
		OutTag = Tags.State_Weather_Storm;
		break;
	default:
		OutTemp = 22.0f;
		OutWind = 10.0f;
		OutTag = Tags.State_Weather_Clear;
		break;
	}
}

bool USBWeatherSubsystem::SaveComponentData_Implementation(UObject* SavePayload)
{
	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (Payload)
	{
		Payload->SerializeObject(GetPathName(), this);
		return true;
	}
	return false;
}

bool USBWeatherSubsystem::LoadComponentData_Implementation(UObject* SavePayload)
{
	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (Payload)
	{
		Payload->DeserializeObject(GetPathName(), this);
		UpdateTimeOfDayState();
		OnWeatherChanged.Broadcast(WeatherState);
		OnHourChanged.Broadcast(TimeOfDay.Hour);
		OnDayChanged.Broadcast(TimeOfDay.Day);
		return true;
	}
	return false;
}
