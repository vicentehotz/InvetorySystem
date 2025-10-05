// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/UniformGridPanel.h"
#include "InventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class EASYINVENTORYSYSTEM_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable)
    void InitializeInventory(UInventoryComponent* InventoryComp);

protected:
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* GridSlots;

private:
    UPROPERTY()
    UInventoryComponent* InventoryRef;

    void RefreshInventory();
};
