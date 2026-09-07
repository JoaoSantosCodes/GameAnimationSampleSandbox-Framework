// Copyright 2026 João Santos. All Rights Reserved.
#include "PCG/SBPCGLootSpawnerSettings.h"
#include "PCGContext.h"
#include "PCGPin.h"
#include "Data/PCGPointData.h"
#include "Metadata/PCGMetadata.h"
#include "Items/SBItemFragment_Placeable.h"
#include "Items/SBItemFragment_WorldActor.h"
#include "Actors/SBBuildingPiece.h"


FString USBPCGLootSpawnerSettings::GetAdditionalTitleInformation() const
{
	if (LootTable)
	{
		return LootTable->GetName();
	}
	return TEXT("No Loot Table");
}

TArray<FPCGPinProperties> USBPCGLootSpawnerSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	FPCGPinProperties& InputPin = Pins.Emplace_GetRef(FName(TEXT("In")), EPCGDataType::Point);
#if WITH_EDITOR
	InputPin.Tooltip = NSLOCTEXT("USBPCGLootSpawnerSettings", "InputPinTooltip", "Input points to process.");
#endif
	return Pins;
}

TArray<FPCGPinProperties> USBPCGLootSpawnerSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	FPCGPinProperties& OutputPin = Pins.Emplace_GetRef(FName(TEXT("Out")), EPCGDataType::Point);
#if WITH_EDITOR
	OutputPin.Tooltip = NSLOCTEXT("USBPCGLootSpawnerSettings", "OutputPinTooltip", "Output points with Class attribute modified.");
#endif
	return Pins;
}

FPCGElementPtr USBPCGLootSpawnerSettings::CreateElement() const
{
	return MakeShared<FSBPCGLootSpawnerElement>();
}

bool FSBPCGLootSpawnerElement::ExecuteInternal(FPCGContext* Context) const
{
	const USBPCGLootSpawnerSettings* Settings = Context->GetInputSettings<USBPCGLootSpawnerSettings>();
	if (!Settings)
	{
		return true;
	}

	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(FName(TEXT("In")));
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	for (const FPCGTaggedData& Input : Inputs)
	{
		const UPCGPointData* PointData = Cast<UPCGPointData>(Input.Data);
		if (!PointData)
		{
			continue;
		}

		UPCGPointData* OutPointData = NewObject<UPCGPointData>();
		OutPointData->InitializeFromData(PointData);

		UPCGMetadata* Metadata = OutPointData->Metadata;
		if (!Metadata)
		{
			continue;
		}

		FName AttributeName = Settings->ClassAttributeName;
		FPCGMetadataAttribute<FString>* StringAttribute = nullptr;
		
		FPCGMetadataAttributeBase* Attribute = Metadata->GetMutableAttribute(AttributeName);
		if (Attribute)
		{
			StringAttribute = static_cast<FPCGMetadataAttribute<FString>*>(Attribute);
		}
		else
		{
			StringAttribute = Metadata->CreateAttribute<FString>(AttributeName, TEXT(""), true, true);
		}

		if (!StringAttribute)
		{
			continue;
		}

		TArray<FPCGPoint>& Points = OutPointData->GetMutablePoints();
		for (FPCGPoint& Point : Points)
		{
			TSubclassOf<AActor> SelectedClass = Settings->DefaultSpawnClass;
			if (Settings->LootTable)
			{
				TArray<FSBLootDropResult> LootResult = Settings->LootTable->RollLoot(1);
				if (LootResult.Num() > 0 && LootResult[0].ItemDefinition)
				{
					TObjectPtr<USBItemDefinition> RolledItem = LootResult[0].ItemDefinition;
					if (Settings->ItemToActorMap.Contains(RolledItem))
					{
						SelectedClass = Settings->ItemToActorMap[RolledItem];
					}
					else if (RolledItem)
					{
						// Fallback 1: Checa se possui fragmento de Ator de Mundo Genérico
						if (const USBItemFragment_WorldActor* WorldActorFrag = Cast<USBItemFragment_WorldActor>(RolledItem->FindFragmentByClass(USBItemFragment_WorldActor::StaticClass())))
						{
							SelectedClass = WorldActorFrag->WorldActorClass;
						}
						// Fallback 2: Checa se possui fragmento de Construção Posicionável
						else if (const USBItemFragment_Placeable* PlaceableFrag = Cast<USBItemFragment_Placeable>(RolledItem->FindFragmentByClass(USBItemFragment_Placeable::StaticClass())))
						{
							SelectedClass = PlaceableFrag->BuildingPieceClass;
						}
					}
				}
			}

			FString ClassPathString = SelectedClass ? SelectedClass->GetPathName() : TEXT("");
			StringAttribute->SetValue(Point.MetadataEntry, ClassPathString);
		}

		FPCGTaggedData& Output = Outputs.Add_GetRef(Input);
		Output.Data = OutPointData;
	}

	return true;
}
