// Copyright Vicente Hotz. All Rights Reserved.

#include "InventoryWidget.h"
#include "InventoryComponent.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

void UInventoryWidget::InitializeWithInventory(UInventoryComponent* InInventory)
{
    InventoryComponent = InInventory;

    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UInventoryWidget::OnInventoryUpdated);
        OnInventoryUpdated();
    }
}

void UInventoryWidget::OnInventoryUpdated()
{
    //TODO Refatorar
    if (!InventoryComponent || !InventoryGrid) return;

    InventoryGrid->ClearChildren();

    const int32 Columns = 5; // Exemplo fixo, pode vir de config

    for (int32 i = 0; i < InventoryComponent->Inventory.Num(); i++)
    {
        const FInventorySlot& InvSlot = InventoryComponent->Inventory[i];

        if (!InvSlot.Item) continue;

        UUserWidget* ItemWidget = CreateWidget<UUserWidget>(this, InventorySlotWidgetClass);
        if (!ItemWidget) continue;

        int32 Row = i / Columns;
        int32 Col = i % Columns;

        UUniformGridSlot* GridSlot = InventoryGrid->AddChildToUniformGrid(ItemWidget, Row, Col);
        if (GridSlot)
        {
            GridSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
            GridSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
        }
    }
}
