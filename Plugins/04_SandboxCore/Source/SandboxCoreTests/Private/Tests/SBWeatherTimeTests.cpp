// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/SBWeatherSubsystem.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBWeatherTimeTestsSpec, "Sandbox.WeatherAndTime", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	USBWeatherSubsystem* WeatherSubsystem;
END_DEFINE_SPEC(FSBWeatherTimeTestsSpec)

void FSBWeatherTimeTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		WeatherSubsystem = TestWorld->GetSubsystem<USBWeatherSubsystem>();
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should advance time of day and trigger day, hour, and period transitions", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// 1. Configura 04:55 da madrugada (Noite)
		WeatherSubsystem->SetDay(1);
		WeatherSubsystem->SetTimeOfDay(4, 55);
		FSBTimeOfDay Time = WeatherSubsystem->GetTimeOfDay();
		TestEqual("Hour should be 4", Time.Hour, 4);
		TestEqual("Minute should be 55", Time.Minute, 55);
		TestEqual("TimePeriod should be Night", Time.TimePeriodTag, Tags.State_Time_Night);

		// 2. Avança 10 minutos -> 05:05 (Amanhecer)
		WeatherSubsystem->ForceAdvanceGameMinutes(10.0f);
		Time = WeatherSubsystem->GetTimeOfDay();
		TestEqual("Hour should be 5", Time.Hour, 5);
		TestEqual("Minute should be 5", Time.Minute, 5);
		TestEqual("TimePeriod should be Dawn", Time.TimePeriodTag, Tags.State_Time_Dawn);
		TestTrue("Should be daytime", Time.bIsDaytime);

		// 3. Avança para 18:05 (Entardecer)
		WeatherSubsystem->SetTimeOfDay(18, 5);
		Time = WeatherSubsystem->GetTimeOfDay();
		TestEqual("TimePeriod should be Dusk", Time.TimePeriodTag, Tags.State_Time_Dusk);
		TestTrue("Should be nighttime/dusk", Time.bIsNighttime);

		// 4. Configura 23:55 e avança 10 minutos -> Virada de Dia (00:05, Dia 2)
		WeatherSubsystem->SetTimeOfDay(23, 55);
		WeatherSubsystem->ForceAdvanceGameMinutes(10.0f);
		Time = WeatherSubsystem->GetTimeOfDay();
		TestEqual("Hour should be 0", Time.Hour, 0);
		TestEqual("Minute should be 5", Time.Minute, 5);
		TestEqual("Day should increment to 2", Time.Day, 2);
		TestEqual("TimePeriod should be Night", Time.TimePeriodTag, Tags.State_Time_Night);
	});

	It("Should transition weather parameters and tags smoothly", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// 1. Configura Clima de Chuva imediato
		WeatherSubsystem->SetWeather(ESBWeatherType::Rain, 0.0f);
		FSBWeatherState State = WeatherSubsystem->GetWeatherState();
		TestEqual("Current weather should be Rain", (int32)State.CurrentWeather, (int32)ESBWeatherType::Rain);
		TestEqual("Weather tag should be Rain", State.WeatherTag, Tags.State_Weather_Rain);
		TestEqual("Temperature should be 16.0", State.AmbientTemperature, 16.0f);

		// 2. Inicia transição para Neve de 5 segundos
		WeatherSubsystem->SetWeather(ESBWeatherType::Snow, 5.0f);

		// 3. Avança 2.5s (metade da transição)
		WeatherSubsystem->Tick(2.5f);
		State = WeatherSubsystem->GetWeatherState();
		TestEqual("Target weather should be Snow", (int32)State.TargetWeather, (int32)ESBWeatherType::Snow);
		TestNearlyEqual("Interpolated temperature at 50% should be approx 5.0", State.AmbientTemperature, 5.0f, 1.0f);
		TestNearlyEqual("Intensity should be 0.5", State.WeatherIntensity, 0.5f, 0.05f);

		// 4. Avança mais 3.0s (concluindo a transição)
		WeatherSubsystem->Tick(3.0f);
		State = WeatherSubsystem->GetWeatherState();
		TestEqual("Current weather should now be Snow", (int32)State.CurrentWeather, (int32)ESBWeatherType::Snow);
		TestEqual("Weather tag should be Snow", State.WeatherTag, Tags.State_Weather_Snow);
		TestEqual("Temperature should be -6.0", State.AmbientTemperature, -6.0f);
	});

	It("Should persist and restore weather and time state through Save and Load", [this]()
	{
		// 1. Define estado customizado no mundo
		WeatherSubsystem->SetDay(7);
		WeatherSubsystem->SetTimeOfDay(15, 45);
		WeatherSubsystem->SetWeather(ESBWeatherType::Thunderstorm, 0.0f);

		// 2. Salva no payload
		USBSavePayload* Payload = NewObject<USBSavePayload>();
		bool bSaved = WeatherSubsystem->SaveComponentData_Implementation(Payload);
		TestTrue("SaveComponentData should succeed", bSaved);

		// 3. Reseta o subsistema para valores padrões
		WeatherSubsystem->SetDay(1);
		WeatherSubsystem->SetTimeOfDay(8, 0);
		WeatherSubsystem->SetWeather(ESBWeatherType::Clear, 0.0f);

		// 4. Carrega do payload
		bool bLoaded = WeatherSubsystem->LoadComponentData_Implementation(Payload);
		TestTrue("LoadComponentData should succeed", bLoaded);

		// 5. Valida os dados restaurados
		FSBTimeOfDay RestoredTime = WeatherSubsystem->GetTimeOfDay();
		TestEqual("Day should be restored to 7", RestoredTime.Day, 7);
		TestEqual("Hour should be restored to 15", RestoredTime.Hour, 15);
		TestEqual("Minute should be restored to 45", RestoredTime.Minute, 45);

		FSBWeatherState RestoredWeather = WeatherSubsystem->GetWeatherState();
		TestEqual("Weather should be restored to Thunderstorm", (int32)RestoredWeather.CurrentWeather, (int32)ESBWeatherType::Thunderstorm);
	});
}
