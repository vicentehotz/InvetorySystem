// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "InventoryComponent.h"

void UInventoryWidget::InitializeInventory(UInventoryComponent* InventoryComp)
{
    InventoryRef = InventoryComp;
    RefreshInventory();
}

void UInventoryWidget::RefreshInventory()
{
    //if (!InventoryRef || !GridSlots) return;

    //GridSlots->ClearChildren();

    //const auto& Items = InventoryRef->GetInventorySlots();
    //for (int32 i = 0; i < Items.Num(); i++)
    //{
    //    // Aqui você criaria widgets de slot (ex: UItemSlotWidget)
    //    // e adicionaria no GridSlots

    //}
}