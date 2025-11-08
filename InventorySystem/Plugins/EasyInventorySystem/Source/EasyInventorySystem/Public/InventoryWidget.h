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
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitializeWithInventory(UInventoryComponent* InInventory);

protected:
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* InventoryGrid;

    UPROPERTY(EditAnywhere, Category = "Inventory")
    TSubclassOf<UUserWidget> InventorySlotWidgetClass;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    UInventoryComponent* InventoryComponent;

    UFUNCTION()
    void OnInventoryUpdated();

private:
    UPROPERTY()
    UInventoryComponent* InventoryRef;

    void RefreshInventory();
};
