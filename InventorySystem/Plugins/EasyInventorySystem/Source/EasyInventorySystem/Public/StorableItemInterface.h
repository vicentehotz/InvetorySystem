// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StorableItemInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UStorableItemInterface : public UInterface
{
    GENERATED_BODY()
};

class EASYINVENTORYSYSTEM_API IStorableItemInterface
{
    GENERATED_BODY()

public:
    // Retorna o DataAsset associado (metadados do item)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    class UItem* GetItemData() const;

    // Retorna a quantidade (stack, munição, etc.)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    int32 GetQuantity();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    class UItem* StoreItem();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
    void OnStored();
};
