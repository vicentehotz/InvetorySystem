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
    Inventory = Items;
}

bool UInventoryComponent::AddItem(UItemData* Item, int32 Quantity)
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

bool UInventoryComponent::RemoveItem(UItemData* Item, int32 Quantity)
{
    if (!Item || Quantity <= 0) return false;

    //FInventorySlot* ItemToBeRemoved = Inventory.FindByKey(Item->Id);
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

void UInventoryComponent::MoveItem(UItemData* Item, FGridPosition NewPosition)
{
    FInventorySlot* ItemToBeMoved = Inventory.FindByKey(Item->Id);
    FInventorySlot* ItemInDesiredPosition = Inventory.FindByKey(NewPosition);

    if (ItemInDesiredPosition)
    {
        ItemInDesiredPosition->Position = ItemToBeMoved->Position;
    }

    ItemToBeMoved->Position = NewPosition;
    OnInventoryUpdated.Broadcast();
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

void UInventoryComponent::ShowInventory()
{
    /*if (!InventoryWidgetClass) return;

    UInventoryWidget* InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
    if (!InventoryWidget) return;

    InventoryWidget->InitializeWithInventory(this);

    InventoryWidget->AddToViewport();*/
    //bShowMouseCursor = true;

    /*FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
    SetInputMode(InputMode);*/
}