// Copyright Under the Bay Studios. All Rights Reserved.

#include "Modes/InventoryLayoutPolicy.h"
#include "InventoryConfig.h"
#include "InventoryItemDefinition.h"

int32 FInventoryLayoutPolicy::StackableRoom(const FInventoryList& List, const UInventoryItemDefinition* Definition) const
{
	if (!Config.bAutoStack) return 0;

	int32 Room = 0;
	for (const FInventoryEntry& Entry : List.Entries)
	{
		if (Entry.Definition == Definition)
		{
			Room += FMath::Max(0, Definition->MaxStackSize - Entry.Quantity);
		}
	}
	return Room;
}

int32 FInventoryLayoutPolicy::FillExistingStacks(FInventoryList& List, UInventoryItemDefinition* Definition, int32 Quantity)
{
	if (!Config.bAutoStack) return Quantity;

	for (FInventoryEntry& Entry : List.Entries)
	{
		if (Quantity <= 0) break;

		if (Entry.Definition == Definition && Entry.Quantity < Definition->MaxStackSize)
		{
			const int32 ToAdd = FMath::Min(Quantity, Definition->MaxStackSize - Entry.Quantity);
			Entry.Quantity += ToAdd;
			Quantity -= ToAdd;
			List.MarkEntryDirty(Entry);
		}
	}
	return Quantity;
}

FInventoryOpResult FInventoryLayoutPolicy::Add(FInventoryList& List, UInventoryItemDefinition* Definition, int32 Quantity, int32& NextEntryId)
{
	if (!Definition || Quantity <= 0)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::InvalidRequest, Quantity);
	}

	const int32 Requested = Quantity;
	Quantity = FillExistingStacks(List, Definition, Quantity);

	while (Quantity > 0)
	{
		FInventoryEntry Placement;
		if (!FindFreePlacement(List, Definition, Placement))
		{
			break;
		}

		Placement.EntryId = NextEntryId++;
		Placement.Definition = Definition;
		Placement.Quantity = FMath::Min(Quantity, Definition->MaxStackSize);
		Quantity -= Placement.Quantity;

		List.AddEntry(Placement);
	}

	if (Quantity == Requested)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::InventoryFull, Quantity);
	}
	if (Quantity > 0)
	{
		return { EInventoryOpStatus::Partial, Quantity };
	}
	return FInventoryOpResult::Ok();
}

FInventoryOpResult FInventoryLayoutPolicy::Remove(FInventoryList& List, UInventoryItemDefinition* Definition, int32 Quantity)
{
	if (!Definition || Quantity <= 0)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::InvalidRequest, Quantity);
	}

	const int32 Requested = Quantity;

	// Drain smallest stacks first so partially-filled stacks are consolidated away.
	TArray<int32> Indices;
	for (int32 i = 0; i < List.Entries.Num(); ++i)
	{
		if (List.Entries[i].Definition == Definition)
		{
			Indices.Add(i);
		}
	}

	if (Indices.Num() == 0)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::ItemNotFound, Quantity);
	}

	Indices.Sort([&List](int32 A, int32 B) { return List.Entries[A].Quantity < List.Entries[B].Quantity; });

	TArray<int32> ToRemove;
	for (int32 Index : Indices)
	{
		if (Quantity <= 0) break;

		FInventoryEntry& Entry = List.Entries[Index];
		const int32 Taken = FMath::Min(Quantity, Entry.Quantity);
		Entry.Quantity -= Taken;
		Quantity -= Taken;

		if (Entry.Quantity <= 0)
		{
			ToRemove.Add(Index);
		}
		else
		{
			List.MarkEntryDirty(Entry);
		}
	}

	// Remove from highest index down so earlier indices stay valid.
	ToRemove.Sort([](int32 A, int32 B) { return A > B; });
	for (int32 Index : ToRemove)
	{
		List.RemoveEntryAt(Index);
	}

	if (Quantity > 0 && Quantity < Requested)
	{
		return { EInventoryOpStatus::Partial, Quantity };
	}
	if (Quantity == Requested)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::ItemNotFound, Quantity);
	}
	return FInventoryOpResult::Ok();
}
