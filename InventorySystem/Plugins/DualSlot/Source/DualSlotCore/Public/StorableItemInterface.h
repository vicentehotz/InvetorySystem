// Copyright Vicente Hotz. All Rights Reserved.

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
    // Retorna o DataAsset associado (metadados do item)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    class UItemData* GetItemData() const;

    // Retorna a quantidade (stack, municao, etc.)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    int32 GetQuantity();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    class UItemData* StoreItem();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
    void OnStored();
};
