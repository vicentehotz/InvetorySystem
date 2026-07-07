// Copyright Under the Bay Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StorableItemInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UStorableItemInterface : public UInterface
{
    GENERATED_BODY()
};

class DUALSLOTCORE_API IStorableItemInterface
{
    GENERATED_BODY()

public:
    // Returns the associated item definition asset (item metadata).
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    class UInventoryItemDefinition* GetItemData() const;

    // Returns the quantity (stack size, ammo count, etc.).
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    int32 GetQuantity();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    class UInventoryItemDefinition* StoreItem();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
    void OnStored();
};
