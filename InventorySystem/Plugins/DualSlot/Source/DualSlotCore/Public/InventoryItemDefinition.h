// Copyright Vicente Hotz. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StorableItemInterface.h"
#include "InventoryItemDefinition.generated.h"

class UInventoryItemFragment;
class UTexture2D;

/**
 * Immutable design-time definition of an item type.
 * Runtime state (quantity, position) lives in FInventoryEntry, never here.
 */
UCLASS(BlueprintType)
class DUALSLOTCORE_API UInventoryItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 1))
	int32 MaxStackSize = 1;

	/** Footprint in grid cells. Ignored in List mode (treated as 1x1). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Grid", meta = (ClampMin = 1))
	FIntPoint Size = FIntPoint(1, 1);

	/** Whether the item may be rotated 90 degrees in Grid mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Grid")
	bool bRotatable = false;

	/** Optional link back to a world representation of this item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TScriptInterface<IStorableItemInterface> StoredItem;

	/** Extension fragments — arbitrary per-item data added by the consuming project. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Item")
	TArray<TObjectPtr<UInventoryItemFragment>> Fragments;

	/** Returns the first fragment of the given class, or null. */
	UFUNCTION(BlueprintCallable, Category = "Item", meta = (DeterminesOutputType = "FragmentClass"))
	UInventoryItemFragment* FindFragmentByClass(TSubclassOf<UInventoryItemFragment> FragmentClass) const;
};
