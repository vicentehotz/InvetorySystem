// Copyright Vicente Hotz. All Rights Reserved.

#include "InventoryComponent.h"
#include "InventoryConfig.h"
#include "InventoryItemDefinition.h"
#include "Modes/ListLayoutPolicy.h"
#include "Modes/GridLayoutPolicy.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

// Defined here so TUniquePtr<FInventoryLayoutPolicy> destroys a complete type.
UInventoryComponent::~UInventoryComponent() = default;

void UInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();
	GetPolicy();
}

void UInventoryComponent::UninitializeComponent()
{
	Policy.Reset();
	Super::UninitializeComponent();
}

FInventoryLayoutPolicy* UInventoryComponent::GetPolicy()
{
	if (Policy)
	{
		return Policy.Get();
	}

	if (!Config)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[DualSlot] %s has no InventoryConfig assigned; using a default List config."),
			*GetPathName());
		Config = NewObject<UInventoryConfig>(this, NAME_None, RF_Transient);
	}

	switch (Config->LayoutMode)
	{
	case EInventoryLayoutMode::Grid:
		Policy = MakeShared<FGridLayoutPolicy>(*Config);
		break;
	case EInventoryLayoutMode::List:
	default:
		Policy = MakeShared<FListLayoutPolicy>(*Config);
		break;
	}
	return Policy.Get();
}

FInventoryOpResult UInventoryComponent::TryAddItem(UInventoryItemDefinition* Definition, int32 Quantity)
{
	const FInventoryOpResult Result = GetPolicy()->Add(InventoryList, Definition, Quantity, NextEntryId);
	if (Result.Status == EInventoryOpStatus::Success || Result.Status == EInventoryOpStatus::Partial)
	{
		OnInventoryUpdated.Broadcast();
	}
	return Result;
}

FInventoryOpResult UInventoryComponent::RemoveItem(UInventoryItemDefinition* Definition, int32 Quantity)
{
	const FInventoryOpResult Result = GetPolicy()->Remove(InventoryList, Definition, Quantity);
	if (Result.Status == EInventoryOpStatus::Success || Result.Status == EInventoryOpStatus::Partial)
	{
		OnInventoryUpdated.Broadcast();
	}
	return Result;
}

FInventoryOpResult UInventoryComponent::MoveEntryToSlot(int32 EntryId, int32 TargetSlot)
{
	const FInventoryOpResult Result = GetPolicy()->MoveToSlot(InventoryList, EntryId, TargetSlot);
	if (Result.Succeeded())
	{
		OnInventoryUpdated.Broadcast();
	}
	return Result;
}

FInventoryOpResult UInventoryComponent::MoveEntryToCell(int32 EntryId, FIntPoint TargetCell, bool bRotated)
{
	const FInventoryOpResult Result = GetPolicy()->MoveToCell(InventoryList, EntryId, TargetCell, bRotated);
	if (Result.Succeeded())
	{
		OnInventoryUpdated.Broadcast();
	}
	return Result;
}

bool UInventoryComponent::CanAccept(UInventoryItemDefinition* Definition, int32 Quantity) const
{
	return GetPolicy()->CanAccept(InventoryList, Definition, Quantity);
}

bool UInventoryComponent::FindEntry(int32 EntryId, FInventoryEntry& OutEntry) const
{
	const int32 Index = InventoryList.IndexOfEntry(EntryId);
	if (Index == INDEX_NONE)
	{
		return false;
	}
	OutEntry = InventoryList.Entries[Index];
	return true;
}

int32 UInventoryComponent::GetItemCount(const UInventoryItemDefinition* Definition) const
{
	int32 Count = 0;
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Definition == Definition)
		{
			Count += Entry.Quantity;
		}
	}
	return Count;
}

UInventoryItemDefinition* UInventoryComponent::FindDefinitionById(FName ItemId) const
{
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Definition && Entry.Definition->Id == ItemId)
		{
			return Entry.Definition;
		}
	}
	return nullptr;
}
