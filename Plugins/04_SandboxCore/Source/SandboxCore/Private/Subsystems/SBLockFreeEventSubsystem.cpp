#include "Subsystems/SBLockFreeEventSubsystem.h"
#include "Misc/ScopeLock.h"

USBLockFreeEventSubsystem::USBLockFreeEventSubsystem()
	: Capacity(1024)
{
}

void USBLockFreeEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeQueue(1024);
}

void USBLockFreeEventSubsystem::Deinitialize()
{
	ClearQueue();
	Super::Deinitialize();
}

void USBLockFreeEventSubsystem::InitializeQueue(int32 InCapacity)
{
	FScopeLock Lock(&QueueLock);
	// A fila em anel tem piso de 16 slots. Clampar em silencio fazia InitializeQueue(10)
	// entregar 16 sem avisar, e qualquer logica de saturacao baseada no valor pedido ficava
	// errada. O piso permanece, mas agora e explicito.
	const int32 MinCapacity = 16;
	if (InCapacity < MinCapacity)
	{
		UE_LOG(LogTemp, Warning, TEXT("SBLockFreeEventSubsystem: capacidade %d solicitada e menor que o minimo %d; usando %d."),
			InCapacity, MinCapacity, MinCapacity);
	}
	Capacity = FMath::Max(MinCapacity, InCapacity);
	RingBuffer.Empty(Capacity);
	RingBuffer.SetNumZeroed(Capacity);

	HeadIndex.Store(0);
	TailIndex.Store(0);
	EnqueuedCount.Store(0);
	DequeuedCount.Store(0);
	DroppedCount.Store(0);
	PendingCount.Store(0);
}

bool USBLockFreeEventSubsystem::EnqueueEvent(const FSBLockFreeEvent& InEvent)
{
	FScopeLock Lock(&QueueLock);
	if (Capacity <= 0 || RingBuffer.Num() == 0)
	{
		InitializeQueue(1024);
	}

	if (PendingCount.Load() >= Capacity)
	{
		DroppedCount++;
		return false;
	}

	int32 Slot = TailIndex.Load() % Capacity;
	RingBuffer[Slot] = InEvent;
	TailIndex.Store((TailIndex.Load() + 1) % Capacity);
	EnqueuedCount++;
	PendingCount++;
	return true;
}

bool USBLockFreeEventSubsystem::DequeueEvent(FSBLockFreeEvent& OutEvent)
{
	FScopeLock Lock(&QueueLock);
	if (PendingCount.Load() <= 0 || Capacity <= 0 || RingBuffer.Num() == 0)
	{
		return false;
	}

	int32 Slot = HeadIndex.Load() % Capacity;
	OutEvent = RingBuffer[Slot];
	HeadIndex.Store((HeadIndex.Load() + 1) % Capacity);
	DequeuedCount++;
	PendingCount--;
	return true;
}

int32 USBLockFreeEventSubsystem::DrainEvents(TArray<FSBLockFreeEvent>& OutEvents, int32 MaxToDrain)
{
	FScopeLock Lock(&QueueLock);
	if (Capacity <= 0 || RingBuffer.Num() == 0)
	{
		return 0;
	}

	int32 Drained = 0;
	while (Drained < MaxToDrain && PendingCount.Load() > 0)
	{
		int32 Slot = HeadIndex.Load() % Capacity;
		OutEvents.Add(RingBuffer[Slot]);
		HeadIndex.Store((HeadIndex.Load() + 1) % Capacity);
		DequeuedCount++;
		PendingCount--;
		Drained++;
	}

	return Drained;
}

FSBLockFreeQueueMetrics USBLockFreeEventSubsystem::GetMetrics() const
{
	FScopeLock Lock(&QueueLock);
	FSBLockFreeQueueMetrics Metrics;
	Metrics.QueueCapacity = Capacity;
	Metrics.EnqueuedEventsCount = EnqueuedCount.Load();
	Metrics.DequeuedEventsCount = DequeuedCount.Load();
	Metrics.DroppedEventsCount = DroppedCount.Load();
	Metrics.PendingCount = PendingCount.Load();
	Metrics.bIsOverflown = (Metrics.DroppedEventsCount > 0);
	return Metrics;
}

int32 USBLockFreeEventSubsystem::GetPendingCount() const
{
	return PendingCount.Load();
}

void USBLockFreeEventSubsystem::ClearQueue()
{
	FScopeLock Lock(&QueueLock);
	HeadIndex.Store(0);
	TailIndex.Store(0);
	PendingCount.Store(0);
	RingBuffer.Empty();
}
