// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Chaos/ChaosEngineInterface.h"
#include "SBAnimNotify_Footstep.generated.h"

class USBSurfaceEffectsDataAsset;

UCLASS(const, hidecategories=Object, CollapseCategories, meta=(DisplayName="Sandbox Footstep Notify"))
class SANDBOXCHARACTER_API USBAnimNotify_Footstep : public UAnimNotify
{
	GENERATED_BODY()

public:
	USBAnimNotify_Footstep();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Footstep")
	FName FootSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Footstep")
	TObjectPtr<USBSurfaceEffectsDataAsset> SurfaceEffectsConfig;
};
