#include "DataAssets/SBSurfaceEffectsDataAsset.h"
#include "Sound/SoundBase.h"

bool USBSurfaceEffectsDataAsset::GetEffectsForSurface(EPhysicalSurface SurfaceType, USoundBase*& OutSound, UObject*& OutVisualEffect) const
{
	OutSound = nullptr;
	OutVisualEffect = nullptr;

	if (const FSBSurfaceEffectConfig* Config = SurfaceEffectsMap.Find(SurfaceType))
	{
		OutSound = Config->Sound;
		OutVisualEffect = Config->VisualEffect;
		return true;
	}

	// Fallback para a superfície padrão (SurfaceType_Default = 0)
	if (const FSBSurfaceEffectConfig* Config = SurfaceEffectsMap.Find(SurfaceType_Default))
	{
		OutSound = Config->Sound;
		OutVisualEffect = Config->VisualEffect;
		return true;
	}

	return false;
}
