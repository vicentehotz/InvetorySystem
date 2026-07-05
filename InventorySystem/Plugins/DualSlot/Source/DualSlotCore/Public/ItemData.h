// Copyright Vicente Hotz. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StorableItemInterface.h"
#include "ItemData.generated.h"

/**
 * Item definition asset. Will evolve into UInventoryItemDefinition in Phase 1.
 */
UCLASS()
class DUALSLOTCORE_API UItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TScriptInterface<IStorableItemInterface> StoredItem;

    UItemData() = default;
};
