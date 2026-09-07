// Copyright 2026 João Santos. All Rights Reserved.
#include "Subsystems/SBLiveConfigSubsystem.h"
#include "Misc/DateTime.h"

USBLiveConfigSubsystem::USBLiveConfigSubsystem()
{
}

void USBLiveConfigSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetSubsystem();
}

void USBLiveConfigSubsystem::Deinitialize()
{
	ResetSubsystem();
	Super::Deinitialize();
}

void USBLiveConfigSubsystem::RegisterSchema(FName SchemaName, const TMap<FName, float>& FloatProps, const TMap<FName, FString>& StringProps)
{
	FSBLiveConfigSchema Schema;
	Schema.SchemaName = SchemaName;
	Schema.Version = 1;
	Schema.LastReloadTicks = FDateTime::UtcNow().GetTicks();

	for (const auto& Pair : FloatProps)
	{
		Schema.Properties.Add(Pair.Key, FSBLiveConfigProperty(Pair.Key, Pair.Value, 1));
	}

	for (const auto& Pair : StringProps)
	{
		Schema.Properties.Add(Pair.Key, FSBLiveConfigProperty(Pair.Key, Pair.Value, 1));
	}

	RegisteredSchemas.Add(SchemaName, Schema);
}

int32 USBLiveConfigSubsystem::HotReloadSchema(FName SchemaName, const TMap<FName, float>& NewFloatProps, const TMap<FName, FString>& NewStringProps)
{
	FSBLiveConfigSchema* Schema = RegisteredSchemas.Find(SchemaName);
	if (!Schema)
	{
		return 0;
	}

	Schema->Version++;
	Schema->LastReloadTicks = FDateTime::UtcNow().GetTicks();
	int32 UpdatedCount = 0;

	for (const auto& Pair : NewFloatProps)
	{
		FSBLiveConfigProperty& Prop = Schema->Properties.FindOrAdd(Pair.Key);
		Prop.PropertyName = Pair.Key;
		Prop.FloatValue = Pair.Value;
		Prop.SchemaVersion = Schema->Version;
		UpdatedCount++;
	}

	for (const auto& Pair : NewStringProps)
	{
		FSBLiveConfigProperty& Prop = Schema->Properties.FindOrAdd(Pair.Key);
		Prop.PropertyName = Pair.Key;
		Prop.StringValue = Pair.Value;
		Prop.SchemaVersion = Schema->Version;
		UpdatedCount++;
	}

	TotalPropertiesUpdated += UpdatedCount;
	TotalHotReloadsExecuted++;

	OnConfigSchemaHotReloaded.Broadcast(SchemaName, Schema->Version);
	return Schema->Version;
}

float USBLiveConfigSubsystem::GetFloatConfig(FName SchemaName, FName PropName, float DefaultVal) const
{
	if (const FSBLiveConfigSchema* Schema = RegisteredSchemas.Find(SchemaName))
	{
		if (const FSBLiveConfigProperty* Prop = Schema->Properties.Find(PropName))
		{
			return Prop->FloatValue;
		}
	}
	return DefaultVal;
}

FString USBLiveConfigSubsystem::GetStringConfig(FName SchemaName, FName PropName, const FString& DefaultVal) const
{
	if (const FSBLiveConfigSchema* Schema = RegisteredSchemas.Find(SchemaName))
	{
		if (const FSBLiveConfigProperty* Prop = Schema->Properties.Find(PropName))
		{
			return Prop->StringValue;
		}
	}
	return DefaultVal;
}

int32 USBLiveConfigSubsystem::GetSchemaVersion(FName SchemaName) const
{
	if (const FSBLiveConfigSchema* Schema = RegisteredSchemas.Find(SchemaName))
	{
		return Schema->Version;
	}
	return 0;
}

FSBLiveConfigMetrics USBLiveConfigSubsystem::GetMetrics() const
{
	FSBLiveConfigMetrics Metrics;
	Metrics.TotalSchemasRegistered = RegisteredSchemas.Num();
	Metrics.TotalHotReloadsExecuted = TotalHotReloadsExecuted;
	Metrics.TotalPropertiesUpdated = TotalPropertiesUpdated;
	Metrics.TotalSubscribedListeners = OnConfigSchemaHotReloaded.IsBound() ? 1 : 0;
	return Metrics;
}

void USBLiveConfigSubsystem::ResetSubsystem()
{
	RegisteredSchemas.Empty();
	TotalHotReloadsExecuted = 0;
	TotalPropertiesUpdated = 0;
}
