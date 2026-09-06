#include "DataAssets/SBCraftingRecipeDataAsset.h"

USBCraftingRecipeDataAsset::USBCraftingRecipeDataAsset()
	: RecipeTag(FGameplayTag::EmptyTag)
	, DisplayName(FText::GetEmpty())
	, RequiredStationTag(FGameplayTag::EmptyTag)
	, ResultItemDef(nullptr)
	, ResultQuantity(1)
	, CraftingDuration(0.0f)
{
}
