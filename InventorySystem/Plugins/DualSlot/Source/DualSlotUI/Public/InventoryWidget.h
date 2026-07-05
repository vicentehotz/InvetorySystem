// Copyright Vicente Hotz. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UUniformGridPanel;

/**
 * Base inventory widget. Listens to UInventoryComponent events; never owned by the component.
 */
UCLASS()
class DUALSLOTUI_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitializeWithInventory(UInventoryComponent* InInventory);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUniformGridPanel> InventoryGrid;

    UPROPERTY(EditAnywhere, Category = "Inventory")
    TSubclassOf<UUserWidget> InventorySlotWidgetClass;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UInventoryComponent> InventoryComponent;

    UFUNCTION()
    void OnInventoryUpdated();
};
