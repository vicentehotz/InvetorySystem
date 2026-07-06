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
        InventoryComponent->OnEntryAdded.AddDynamic(this, &UInventoryWidget::HandleEntryAdded);
        InventoryComponent->OnEntryChanged.AddDynamic(this, &UInventoryWidget::HandleEntryChanged);
        InventoryComponent->OnEntryRemoved.AddDynamic(this, &UInventoryWidget::HandleEntryRemoved);
        InventoryComponent->OnOperationRejected.AddDynamic(this, &UInventoryWidget::HandleOperationRejected);
        OnInventoryUpdated();
    }
}

void UInventoryWidget::HandleEntryAdded(const FInventoryEntry& Entry) { OnEntryAddedBP(Entry); }
void UInventoryWidget::HandleEntryChanged(const FInventoryEntry& Entry) { OnEntryChangedBP(Entry); }
void UInventoryWidget::HandleEntryRemoved(const FInventoryEntry& Entry) { OnEntryRemovedBP(Entry); }
void UInventoryWidget::HandleOperationRejected(const FInventoryOpResult& Result) { OnOperationRejectedBP(Result); }

void UInventoryWidget::OnInventoryUpdated()
{
    // Fallback full-refresh renderer for simple List UIs. Grid/custom UIs should
    // prefer the granular OnEntry*BP hooks instead of relying on this.
    if (!InventoryComponent || !InventoryGrid) return;

    InventoryGrid->ClearChildren();

    const int32 Columns = 5; // Exemplo fixo, pode vir de config

    const TArray<FInventoryEntry> Entries = InventoryComponent->GetEntries();
    for (int32 i = 0; i < Entries.Num(); i++)
    {
        const FInventoryEntry& Entry = Entries[i];

        if (!Entry.Definition) continue;

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
