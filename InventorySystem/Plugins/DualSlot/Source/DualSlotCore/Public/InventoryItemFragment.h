// Copyright Under the Bay Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryItemFragment.generated.h"

/**
 * Extension point for item definitions. Subclass in C++ or Blueprint to attach
 * arbitrary data/behavior (consumable stats, equip info, crafting recipe, ...)
 * to a UInventoryItemDefinition without touching the plugin.
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, BlueprintType, Blueprintable)
class DUALSLOTCORE_API UInventoryItemFragment : public UObject
{
	GENERATED_BODY()
};
