// Copyright Vicente Hotz. All Rights Reserved.

#include "Modes/GridLayoutPolicy.h"
#include "InventoryConfig.h"
#include "InventoryItemDefinition.h"

bool FGridLayoutPolicy::Overlaps(FIntPoint TopLeftA, FIntPoint FootprintA, FIntPoint TopLeftB, FIntPoint FootprintB)
{
	return TopLeftA.X < TopLeftB.X + FootprintB.X
		&& TopLeftB.X < TopLeftA.X + FootprintA.X
		&& TopLeftA.Y < TopLeftB.Y + FootprintB.Y
		&& TopLeftB.Y < TopLeftA.Y + FootprintA.Y;
}

bool FGridLayoutPolicy::IsAreaFree(const FInventoryList& List, FIntPoint TopLeft, FIntPoint Footprint, int32 IgnoreEntryId) const
{
	if (TopLeft.X < 0 || TopLeft.Y < 0
		|| TopLeft.X + Footprint.X > Config.GridSize.X
		|| TopLeft.Y + Footprint.Y > Config.GridSize.Y)
	{
		return false;
	}

	for (const FInventoryEntry& Entry : List.Entries)
	{
		if (Entry.EntryId == IgnoreEntryId) continue;

		if (Overlaps(TopLeft, Footprint, Entry.TopLeft, Entry.GetFootprint()))
		{
			return false;
		}
	}
	return true;
}

bool FGridLayoutPolicy::FindFreePlacement(const FInventoryList& List, const UInventoryItemDefinition* Definition, FInventoryEntry& OutPlacement) const
{
	const FIntPoint BaseFootprint = Definition->Size;

	for (int32 Y = 0; Y < Config.GridSize.Y; ++Y)
	{
		for (int32 X = 0; X < Config.GridSize.X; ++X)
		{
			const FIntPoint Cell(X, Y);

			if (IsAreaFree(List, Cell, BaseFootprint))
			{
				OutPlacement.TopLeft = Cell;
				OutPlacement.bRotated = false;
				return true;
			}

			if (Definition->bRotatable && BaseFootprint.X != BaseFootprint.Y
				&& IsAreaFree(List, Cell, FIntPoint(BaseFootprint.Y, BaseFootprint.X)))
			{
				OutPlacement.TopLeft = Cell;
				OutPlacement.bRotated = true;
				return true;
			}
		}
	}
	return false;
}

bool FGridLayoutPolicy::CanAccept(const FInventoryList& List, const UInventoryItemDefinition* Definition, int32 Quantity) const
{
	if (!Definition || Quantity <= 0) return false;

	const int32 Room = StackableRoom(List, Definition);
	if (Quantity <= Room) return true;

	// Simulate placement of the new stacks on a scratch copy (placements interact,
	// so free-cell counting is not sufficient for multi-cell items).
	int32 Left = Quantity - Room;
	FInventoryList Scratch;
	Scratch.Entries = List.Entries;

	int32 ScratchId = -1000; // synthetic ids, never persisted
	while (Left > 0)
	{
		FInventoryEntry Placement;
		if (!FindFreePlacement(Scratch, Definition, Placement))
		{
			return false;
		}
		Placement.EntryId = ScratchId--;
		Placement.Definition = const_cast<UInventoryItemDefinition*>(Definition);
		Placement.Quantity = FMath::Min(Left, Definition->MaxStackSize);
		Left -= Placement.Quantity;
		Scratch.Entries.Add(Placement);
	}
	return true;
}

FInventoryOpResult FGridLayoutPolicy::MoveToCell(FInventoryList& List, int32 EntryId, FIntPoint TargetCell, bool bRotated)
{
	FInventoryEntry* Entry = List.FindEntryById(EntryId);
	if (!Entry)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::ItemNotFound);
	}
	if (bRotated && Entry->Definition && !Entry->Definition->bRotatable)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::InvalidRequest);
	}

	const FIntPoint Footprint = bRotated && Entry->Definition
		? FIntPoint(Entry->Definition->Size.Y, Entry->Definition->Size.X)
		: (Entry->Definition ? Entry->Definition->Size : FIntPoint(1, 1));

	if (TargetCell.X < 0 || TargetCell.Y < 0
		|| TargetCell.X + Footprint.X > Config.GridSize.X
		|| TargetCell.Y + Footprint.Y > Config.GridSize.Y)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::OutOfBounds);
	}

	if (!IsAreaFree(List, TargetCell, Footprint, EntryId))
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::PositionOccupied);
	}

	Entry->TopLeft = TargetCell;
	Entry->bRotated = bRotated;
	List.MarkEntryDirty(*Entry);
	return FInventoryOpResult::Ok();
}
