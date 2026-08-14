#include "DataAssets/SBPrimaryDataAsset.h"

FPrimaryAssetId USBPrimaryDataAsset::GetPrimaryAssetId() const
{
	// Return a clean Primary Asset ID based on class name and object name
	return FPrimaryAssetId(GetClass()->GetFName(), GetFName());
}
