// Copyright Under the Bay Studios. All Rights Reserved.

#include "Modes/ListLayoutPolicy.h"
#include "InventoryConfig.h"
#include "InventoryItemDefinition.h"

int32 FListLayoutPolicy::FindFreeSlot(const FInventoryList& List) const
{
	for (int32 Slot = 0; Slot < Config.MaxSlots; ++Slot)
	{
		const bool bOccupied = List.Entries.ContainsByPredicate(
			[Slot](const FInventoryEntry& E) { return E.SlotIndex == Slot; });

		if (!bOccupied)
		{
			return Slot;
		}
	}
	return INDEX_NONE;
}

int32 FListLayoutPolicy::CountFreeSlots(const FInventoryList& List) const
{
	return FMath::Max(0, Config.MaxSlots - List.Entries.Num());
}

bool FListLayoutPolicy::FindFreePlacement(const FInventoryList& List, const UInventoryItemDefinition* Definition, FInventoryEntry& OutPlacement) const
{
	const int32 Slot = FindFreeSlot(List);
	if (Slot == INDEX_NONE)
	{
		return false;
	}
	OutPlacement.SlotIndex = Slot;
	return true;
}

bool FListLayoutPolicy::CanAccept(const FInventoryList& List, const UInventoryItemDefinition* Definition, int32 Quantity) const
{
	if (!Definition || Quantity <= 0) return false;

	const int32 Room = StackableRoom(List, Definition);
	if (Quantity <= Room) return true;

	const int32 NewStacksNeeded = FMath::DivideAndRoundUp(Quantity - Room, Definition->MaxStackSize);
	return NewStacksNeeded <= CountFreeSlots(List);
}

FInventoryOpResult FListLayoutPolicy::MoveToSlot(FInventoryList& List, int32 EntryId, int32 TargetSlot)
{
	if (TargetSlot < 0 || TargetSlot >= Config.MaxSlots)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::OutOfBounds);
	}

	FInventoryEntry* Entry = List.FindEntryById(EntryId);
	if (!Entry)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::ItemNotFound);
	}

	FInventoryEntry* Occupant = List.Entries.FindByPredicate(
		[TargetSlot](const FInventoryEntry& E) { return E.SlotIndex == TargetSlot; });

	if (Occupant && Occupant->EntryId != EntryId)
	{
		// Swap positions with the occupant.
		Occupant->SlotIndex = Entry->SlotIndex;
		List.MarkEntryDirty(*Occupant);
	}

	Entry->SlotIndex = TargetSlot;
	List.MarkEntryDirty(*Entry);
	return FInventoryOpResult::Ok();
}
