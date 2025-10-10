// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInventoryComponent::InitializeInventory(TArray<FInventorySlot> Items)
{
    //TODO
}

bool UInventoryComponent::AddItem(UItemData* Item, int32 Quantity)
{
    //TODO Refactor
    if (!Item || Quantity <= 0) return false;

    // Verifica se item já existe e pode empilhar
    for (FInventorySlot& Slot : Inventory)
    {
        if (Slot.Item == Item && Slot.Quantity < Item->MaxStackSize)
        {
            int32 SpaceLeft = Item->MaxStackSize - Slot.Quantity;
            int32 ToAdd = FMath::Min(SpaceLeft, Quantity);
            Slot.Quantity += ToAdd;
            Quantity -= ToAdd;
            if (Quantity <= 0) return true;
        }
    }

    // Adiciona novos slots
    while (Quantity > 0 && Inventory.Num() < MaxSlots)
    {
        int32 ToAdd = FMath::Min(Quantity, Item->MaxStackSize);
        Inventory.Add(FInventorySlot{ Item, ToAdd });
        Quantity -= ToAdd;
    }

    return Quantity <= 0;
}

bool UInventoryComponent::RemoveItem(UItemData* Item, int32 Quantity)
{
    //TODO Refactor
    if (!Item || Quantity <= 0) return false;

    for (int32 i = Inventory.Num() - 1; i >= 0; --i)
    {
        FInventorySlot& Slot = Inventory[i];
        if (Slot.Item == Item)
        {
            int32 ToRemove = FMath::Min(Slot.Quantity, Quantity);
            Slot.Quantity -= ToRemove;
            Quantity -= ToRemove;

            if (Slot.Quantity <= 0)
                Inventory.RemoveAt(i);

            if (Quantity <= 0)
                return true;
        }
    }

    return Quantity <= 0;
}

void UInventoryComponent::MoveItem(UItemData* Item, FGridPosition NewPosition)
{
    FInventorySlot* ItemToBeMoved = Inventory.FindByKey(Item->Id);
    FInventorySlot* ItemInDesiredPosition = Inventory.FindByKey(NewPosition);

    if (ItemInDesiredPosition)
    {
        ItemInDesiredPosition->Position = ItemToBeMoved->Position;
    }

    ItemToBeMoved->Position = NewPosition;
}

UItemData* UInventoryComponent::GetItemById(FName ItemId)
{
    FInventorySlot* Slot = Inventory.FindByKey(ItemId);

    if (Slot)
    {
        return Slot->Item;
    }

    UE_LOG(LogTemp, Warning, TEXT("Item with id %s not found"), *ItemId.ToString());

    return nullptr;
}

int32 UInventoryComponent::GetItemCount(UItemData* Item) const
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
