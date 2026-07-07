// Copyright Under the Bay Studios. All Rights Reserved.

#include "InventoryWidget.h"
#include "InventoryComponent.h"
#include "InventoryConfig.h"
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

    const bool bIsGridMode = InventoryComponent->Config
        && InventoryComponent->Config->LayoutMode == EInventoryLayoutMode::Grid;

    // List mode has no natural column count; pick a squarish default from capacity.
    const int32 ListColumns = InventoryComponent->Config
        ? FMath::Max(1, FMath::CeilToInt(FMath::Sqrt(static_cast<float>(InventoryComponent->Config->MaxSlots))))
        : 5;

    const TArray<FInventoryEntry> Entries = InventoryComponent->GetEntries();
    for (int32 i = 0; i < Entries.Num(); i++)
    {
        const FInventoryEntry& Entry = Entries[i];

        if (!Entry.Definition) continue;

        UUserWidget* ItemWidget = CreateWidget<UUserWidget>(this, InventorySlotWidgetClass);
        if (!ItemWidget) continue;

        // Grid mode: honor the entry's real cell so spatial placement is visible.
        // List mode: no inherent 2D position, so lay slots out left-to-right, wrapping.
        const int32 Row = bIsGridMode ? Entry.TopLeft.Y : (i / ListColumns);
        const int32 Col = bIsGridMode ? Entry.TopLeft.X : (i % ListColumns);

        UUniformGridSlot* GridSlot = InventoryGrid->AddChildToUniformGrid(ItemWidget, Row, Col);
        if (GridSlot)
        {
            GridSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
            GridSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
        }
    }
}
