// Copyright 2026 João Santos. All Rights Reserved.
#include "Subsystems/SBVisualDebuggerSubsystem.h"

USBVisualDebuggerSubsystem::USBVisualDebuggerSubsystem()
{
}

void USBVisualDebuggerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetSubsystem();
}

void USBVisualDebuggerSubsystem::Deinitialize()
{
	ResetSubsystem();
	Super::Deinitialize();
}

void USBVisualDebuggerSubsystem::SetCategoryEnabled(ESBOverlayCategory Category, bool bEnabled)
{
	if (Category == ESBOverlayCategory::All)
	{
		EnabledCategoriesBitmask = bEnabled ? 0xFF : 0;
	}
	else
	{
		uint32 Mask = (1 << static_cast<uint8>(Category));
		if (bEnabled)
		{
			EnabledCategoriesBitmask |= Mask;
		}
		else
		{
			EnabledCategoriesBitmask &= ~Mask;
		}
	}

	OnOverlayCategoryToggled.Broadcast(Category, bEnabled);
}

bool USBVisualDebuggerSubsystem::IsCategoryEnabled(ESBOverlayCategory Category) const
{
	if (Category == ESBOverlayCategory::All)
	{
		return EnabledCategoriesBitmask != 0;
	}

	uint32 Mask = (1 << static_cast<uint8>(Category));
	return (EnabledCategoriesBitmask & Mask) != 0;
}

void USBVisualDebuggerSubsystem::QueueRenderItem(const FSBOverlayRenderItem& Item)
{
	if (IsCategoryEnabled(Item.Category))
	{
		QueuedRenderItems.Add(Item);
	}
}

TArray<FSBOverlayRenderItem> USBVisualDebuggerSubsystem::GetQueuedRenderItems(ESBOverlayCategory Category) const
{
	if (Category == ESBOverlayCategory::All)
	{
		return QueuedRenderItems;
	}

	TArray<FSBOverlayRenderItem> Filtered;
	for (const FSBOverlayRenderItem& Item : QueuedRenderItems)
	{
		if (Item.Category == Category)
		{
			Filtered.Add(Item);
		}
	}
	return Filtered;
}

void USBVisualDebuggerSubsystem::ClearRenderItems()
{
	QueuedRenderItems.Empty();
}

FSBVisualDebuggerMetrics USBVisualDebuggerSubsystem::GetMetrics() const
{
	FSBVisualDebuggerMetrics Metrics;
	Metrics.TotalRenderItemsQueued = QueuedRenderItems.Num();
	Metrics.EnabledCategoriesMask = static_cast<int32>(EnabledCategoriesBitmask);

	int32 ActiveCount = 0;
	uint32 Temp = EnabledCategoriesBitmask;
	while (Temp > 0)
	{
		if (Temp & 1) ActiveCount++;
		Temp >>= 1;
	}
	Metrics.ActiveOverlaysCount = ActiveCount;

	return Metrics;
}

void USBVisualDebuggerSubsystem::ResetSubsystem()
{
	EnabledCategoriesBitmask = 0;
	QueuedRenderItems.Empty();
}
