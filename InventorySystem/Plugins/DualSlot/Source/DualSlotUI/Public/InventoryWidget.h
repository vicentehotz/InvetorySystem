// Copyright Vicente Hotz. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryList.h"
#include "InventoryOpResult.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UUniformGridPanel;

/**
 * Base inventory widget. Listens to UInventoryComponent events; never owned by the component.
 * Subclass in Blueprint and implement the BlueprintImplementableEvent hooks to render
 * List or Grid layouts incrementally; OnInventoryUpdated remains for simple full-refresh UI.
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

    UFUNCTION()
    void HandleEntryAdded(const FInventoryEntry& Entry);

    UFUNCTION()
    void HandleEntryChanged(const FInventoryEntry& Entry);

    UFUNCTION()
    void HandleEntryRemoved(const FInventoryEntry& Entry);

    UFUNCTION()
    void HandleOperationRejected(const FInventoryOpResult& Result);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
    void OnEntryAddedBP(const FInventoryEntry& Entry);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
    void OnEntryChangedBP(const FInventoryEntry& Entry);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
    void OnEntryRemovedBP(const FInventoryEntry& Entry);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
    void OnOperationRejectedBP(const FInventoryOpResult& Result);
};
