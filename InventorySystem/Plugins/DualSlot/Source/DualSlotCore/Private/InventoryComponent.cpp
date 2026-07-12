// Copyright Under the Bay Studios. All Rights Reserved.

#include "InventoryComponent.h"
#include "DualSlotCore.h"
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
		UE_LOG(LogDualSlot, Warning,
			TEXT("%s has no InventoryConfig assigned; using a default List config."),
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

FInventoryOpResult UInventoryComponent::ApplyMutation(TFunctionRef<FInventoryOpResult()> Mutator)
{
	TMap<int32, FInventoryEntry> Before;
	Before.Reserve(InventoryList.Entries.Num());
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		Before.Add(Entry.EntryId, Entry);
	}

	const FInventoryOpResult Result = Mutator();

	TSet<int32> SeenAfter;
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		SeenAfter.Add(Entry.EntryId);

		if (const FInventoryEntry* Prior = Before.Find(Entry.EntryId))
		{
			if (Prior->Quantity != Entry.Quantity || Prior->SlotIndex != Entry.SlotIndex
				|| Prior->TopLeft != Entry.TopLeft || Prior->bRotated != Entry.bRotated)
			{
				OnEntryChanged.Broadcast(Entry);
			}
		}
		else
		{
			OnEntryAdded.Broadcast(Entry);
		}
	}

	for (const auto& Pair : Before)
	{
		if (!SeenAfter.Contains(Pair.Key))
		{
			OnEntryRemoved.Broadcast(Pair.Value);
		}
	}

	if (Result.Status != EInventoryOpStatus::Success)
	{
		OnOperationRejected.Broadcast(Result);
	}
	if (Result.Status == EInventoryOpStatus::Success || Result.Status == EInventoryOpStatus::Partial)
	{
		OnInventoryUpdated.Broadcast();
	}
	return Result;
}

FInventoryOpResult UInventoryComponent::TryAddItem(UInventoryItemDefinition* Definition, int32 Quantity)
{
	return ApplyMutation([&]() { return GetPolicy()->Add(InventoryList, Definition, Quantity, NextEntryId); });
}

FInventoryOpResult UInventoryComponent::RemoveItem(UInventoryItemDefinition* Definition, int32 Quantity)
{
	return ApplyMutation([&]() { return GetPolicy()->Remove(InventoryList, Definition, Quantity); });
}

FInventoryOpResult UInventoryComponent::ExecuteExchange(const TArray<FInventoryExchangeItem>& Consume, const TArray<FInventoryExchangeItem>& Produce)
{
	// Validate up front so no work (and no mutation/events) happens on a malformed request.
	if (Consume.Num() == 0 && Produce.Num() == 0)
	{
		const FInventoryOpResult Result = FInventoryOpResult::Fail(EInventoryOpStatus::InvalidRequest);
		OnOperationRejected.Broadcast(Result);
		return Result;
	}
	for (const FInventoryExchangeItem& Item : Consume)
	{
		if (!Item.Definition || Item.Quantity <= 0)
		{
			const FInventoryOpResult Result = FInventoryOpResult::Fail(EInventoryOpStatus::InvalidRequest);
			OnOperationRejected.Broadcast(Result);
			return Result;
		}
	}
	for (const FInventoryExchangeItem& Item : Produce)
	{
		if (!Item.Definition || Item.Quantity <= 0)
		{
			const FInventoryOpResult Result = FInventoryOpResult::Fail(EInventoryOpStatus::InvalidRequest);
			OnOperationRejected.Broadcast(Result);
			return Result;
		}
	}

	// Aggregate duplicate Consume definitions so partial-drain ordering can't double-remove.
	TArray<FInventoryExchangeItem> AggregatedConsume;
	for (const FInventoryExchangeItem& Item : Consume)
	{
		FInventoryExchangeItem* Existing = AggregatedConsume.FindByPredicate(
			[&Item](const FInventoryExchangeItem& E) { return E.Definition == Item.Definition; });
		if (Existing)
		{
			Existing->Quantity += Item.Quantity;
		}
		else
		{
			AggregatedConsume.Add(Item);
		}
	}

	return ApplyMutation([&]() -> FInventoryOpResult
	{
		// Snapshot for exact rollback. NextEntryId must be restored too so a rolled-back
		// production doesn't burn ids and produce a "changed" phantom on a later op.
		const TArray<FInventoryEntry> BackupEntries = InventoryList.Entries;
		const int32 BackupNextEntryId = NextEntryId;

		auto Rollback = [&]()
		{
			InventoryList.Entries = BackupEntries;
			NextEntryId = BackupNextEntryId;
			// FastArray internal ReplicationID/Map state may reference now-removed items;
			// MarkArrayDirty() is the documented-safe blunt reset after a raw restore.
			InventoryList.MarkArrayDirty();
		};

		// Removes first, so outputs can occupy space freed by inputs (the swap case).
		for (const FInventoryExchangeItem& Item : AggregatedConsume)
		{
			const FInventoryOpResult Result = GetPolicy()->Remove(InventoryList, Item.Definition, Item.Quantity);
			if (Result.Status != EInventoryOpStatus::Success) // Partial counts as failure: all-or-nothing.
			{
				Rollback();
				return Result;
			}
		}

		// Then produce all outputs.
		for (const FInventoryExchangeItem& Item : Produce)
		{
			const FInventoryOpResult Result = GetPolicy()->Add(InventoryList, Item.Definition, Item.Quantity, NextEntryId);
			if (Result.Status != EInventoryOpStatus::Success)
			{
				Rollback();
				return Result;
			}
		}

		// Success: policies already mark items/array dirty per-op, no extra dirtying needed.
		return FInventoryOpResult::Ok();
	});
}

FInventoryOpResult UInventoryComponent::MoveEntryToSlot(int32 EntryId, int32 TargetSlot)
{
	return ApplyMutation([&]() { return GetPolicy()->MoveToSlot(InventoryList, EntryId, TargetSlot); });
}

FInventoryOpResult UInventoryComponent::MoveEntryToCell(int32 EntryId, FIntPoint TargetCell, bool bRotated)
{
	return ApplyMutation([&]() { return GetPolicy()->MoveToCell(InventoryList, EntryId, TargetCell, bRotated); });
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
