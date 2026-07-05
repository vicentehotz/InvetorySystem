// Copyright Vicente Hotz. All Rights Reserved.

#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::InitializeInventory(TArray<FInventorySlot> Items)
{
    Inventory = Items;
    OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::AddItem(UInventoryItemDefinition* Item, int32 Quantity)
{
    if (!Item || Quantity <= 0) return false;

    FInventorySlot* ItemToBeAdded = Inventory.FindByKey(Item->Id);

    if (ItemToBeAdded)
    {
        int32 NewQuantity = ItemToBeAdded->Quantity + Quantity;

        bool HasExceededMaxStack = NewQuantity > ItemToBeAdded->Item->MaxStackSize;

        if (!HasExceededMaxStack)
        {
            ItemToBeAdded->Quantity = NewQuantity;
            OnInventoryUpdated.Broadcast();
            return true;
        }

        if (Inventory.Num() >= MaxSlots)
        {
            UE_LOG(LogTemp, Warning, TEXT("There's no inventory space"));
            return false;
        }

        while (Quantity > 0 && Inventory.Num() < MaxSlots)
        {
            int32 ToAdd = FMath::Min(Quantity, Item->MaxStackSize);
            Inventory.Add(FInventorySlot{ Item, ToAdd });
            Quantity -= ToAdd;
        }
    }
    else
    {
        while (Quantity > 0 && Inventory.Num() < MaxSlots)
        {
            int32 ToAdd = FMath::Min(Quantity, Item->MaxStackSize);
            Inventory.Add(FInventorySlot{ Item, ToAdd });
            Quantity -= ToAdd;
        }
    }

    OnInventoryUpdated.Broadcast();
    return true;
}

bool UInventoryComponent::RemoveItem(UInventoryItemDefinition* Item, int32 Quantity)
{
    if (!Item || Quantity <= 0) return false;

    int32 Index = Inventory.IndexOfByKey(Item->Id);

    if (Index < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Item not found"));
        return false;
    }

    int32 NewQuantity = Inventory[Index].Quantity - Quantity;

    if (NewQuantity <= 0)
    {
        Inventory.RemoveAt(Index);
        OnInventoryUpdated.Broadcast();
        return true;
    }

    Inventory[Index].Quantity = NewQuantity;
    OnInventoryUpdated.Broadcast();
    return true;
}

void UInventoryComponent::MoveItem(UInventoryItemDefinition* Item, FGridPosition NewPosition)
{
    if (!Item) return;

    FInventorySlot* ItemToBeMoved = Inventory.FindByKey(Item->Id);
    if (!ItemToBeMoved) return;

    FInventorySlot* ItemInDesiredPosition = Inventory.FindByKey(NewPosition);

    if (ItemInDesiredPosition)
    {
        ItemInDesiredPosition->Position = ItemToBeMoved->Position;
    }

    ItemToBeMoved->Position = NewPosition;
    OnInventoryUpdated.Broadcast();
}

UInventoryItemDefinition* UInventoryComponent::GetItemById(FName ItemId)
{
    FInventorySlot* Slot = Inventory.FindByKey(ItemId);

    if (Slot)
    {
        return Slot->Item;
    }

    UE_LOG(LogTemp, Warning, TEXT("Item with id %s not found"), *ItemId.ToString());

    return nullptr;
}

int32 UInventoryComponent::GetItemCount(UInventoryItemDefinition* Item) const
{
    int32 Count = 0;
    for (const FInventorySlot& Slot : Inventory)
    {
        if (Slot.Item == Item)
            Count += Slot.Quantity;
    }
    return Count;
}

TArray<FInventorySlot> UInventoryComponent::GetInventory()
{
    return Inventory;
}
