#pragma once

#include "CoreMinimal.h"
#include "SBPersistenceTypes.generated.h"

USTRUCT(BlueprintType)
struct SANDBOXCORE_API FSBEntityId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
	FGuid Guid;

	bool IsValid() const { return Guid.IsValid(); }
	void Generate() { Guid = FGuid::NewGuid(); }
	void Reset() { Guid.Invalidate(); }

	bool operator==(const FSBEntityId& Other) const { return Guid == Other.Guid; }
	bool operator!=(const FSBEntityId& Other) const { return Guid != Other.Guid; }
};

USTRUCT(BlueprintType)
struct SANDBOXCORE_API FSBSerializedActorData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Persistence")
	FGuid Guid;

	UPROPERTY(BlueprintReadOnly, Category = "Persistence")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(BlueprintReadOnly, Category = "Persistence")
	FTransform Transform;

	UPROPERTY(BlueprintReadOnly, Category = "Persistence")
	TArray<uint8> BytePayload;

	UPROPERTY(BlueprintReadOnly, Category = "Persistence")
	bool bWasDestroyed = false;
};
