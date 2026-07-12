// Copyright Under the Bay Studios. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "UObject/Package.h"
#include "Modes/ListLayoutPolicy.h"
#include "Modes/GridLayoutPolicy.h"
#include "InventoryComponent.h"
#include "InventoryConfig.h"
#include "InventoryItemDefinition.h"
#include "InventoryList.h"
#include "InventoryOpResult.h"

namespace DualSlotTestUtil
{
	static UInventoryConfig* MakeListConfig(int32 MaxSlots, bool bAutoStack = true)
	{
		UInventoryConfig* Config = NewObject<UInventoryConfig>(GetTransientPackage());
		Config->LayoutMode = EInventoryLayoutMode::List;
		Config->MaxSlots = MaxSlots;
		Config->bAutoStack = bAutoStack;
		return Config;
	}

	static UInventoryConfig* MakeGridConfig(FIntPoint GridSize, bool bAutoStack = true)
	{
		UInventoryConfig* Config = NewObject<UInventoryConfig>(GetTransientPackage());
		Config->LayoutMode = EInventoryLayoutMode::Grid;
		Config->GridSize = GridSize;
		Config->bAutoStack = bAutoStack;
		return Config;
	}

	static UInventoryItemDefinition* MakeItem(int32 MaxStackSize, FIntPoint Size = FIntPoint(1, 1), bool bRotatable = false)
	{
		UInventoryItemDefinition* Item = NewObject<UInventoryItemDefinition>(GetTransientPackage());
		Item->MaxStackSize = MaxStackSize;
		Item->Size = Size;
		Item->bRotatable = bRotatable;
		return Item;
	}

	static UInventoryComponent* MakeComponent(UInventoryConfig* Config)
	{
		UInventoryComponent* Component = NewObject<UInventoryComponent>(GetTransientPackage());
		Component->Config = Config;
		return Component;
	}

	static FInventoryExchangeItem Exchange(UInventoryItemDefinition* Definition, int32 Quantity)
	{
		FInventoryExchangeItem Item;
		Item.Definition = Definition;
		Item.Quantity = Quantity;
		return Item;
	}

	/** Stable, order-independent snapshot of contents for before/after equality checks. */
	static TMap<int32, int32> ContentCounts(const TArray<FInventoryEntry>& Entries)
	{
		TMap<int32, int32> Counts;
		for (const FInventoryEntry& Entry : Entries)
		{
			Counts.FindOrAdd(Entry.EntryId) = Entry.Quantity;
		}
		return Counts;
	}
}

// ---------------------------------------------------------------------------
// List mode
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListStackingMergeTest, "DualSlot.Policies.List.StackingMerge",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FListStackingMergeTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeListConfig(10);
	UInventoryItemDefinition* Item = MakeItem(10);
	FListLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	FInventoryOpResult First = Policy.Add(List, Item, 4, NextEntryId);
	TestEqual(TEXT("First add succeeds"), First.Status, EInventoryOpStatus::Success);
	TestEqual(TEXT("One entry after first add"), List.Entries.Num(), 1);

	FInventoryOpResult Second = Policy.Add(List, Item, 3, NextEntryId);
	TestEqual(TEXT("Second add merges into stack"), Second.Status, EInventoryOpStatus::Success);
	TestEqual(TEXT("Still one entry"), List.Entries.Num(), 1);
	TestEqual(TEXT("Stack quantity merged"), List.Entries[0].Quantity, 7);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListOverflowMultipleStacksTest, "DualSlot.Policies.List.OverflowCreatesMultipleStacks",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FListOverflowMultipleStacksTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeListConfig(10);
	UInventoryItemDefinition* Item = MakeItem(5);
	FListLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	FInventoryOpResult Result = Policy.Add(List, Item, 12, NextEntryId);
	TestEqual(TEXT("Add succeeds"), Result.Status, EInventoryOpStatus::Success);
	TestEqual(TEXT("Three stacks created"), List.Entries.Num(), 3);
	TestEqual(TEXT("Stack 0 full"), List.Entries[0].Quantity, 5);
	TestEqual(TEXT("Stack 1 full"), List.Entries[1].Quantity, 5);
	TestEqual(TEXT("Stack 2 remainder"), List.Entries[2].Quantity, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListInventoryFullTest, "DualSlot.Policies.List.InventoryFullWhenSlotsExhausted",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FListInventoryFullTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeListConfig(2, false);
	UInventoryItemDefinition* Item = MakeItem(1);
	FListLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	Policy.Add(List, Item, 1, NextEntryId);
	Policy.Add(List, Item, 1, NextEntryId);
	FInventoryOpResult Result = Policy.Add(List, Item, 1, NextEntryId);

	TestEqual(TEXT("Add fails with InventoryFull"), Result.Status, EInventoryOpStatus::InventoryFull);
	TestEqual(TEXT("Full quantity remaining"), Result.QuantityRemaining, 1);
	TestEqual(TEXT("No new entry added"), List.Entries.Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListPartialAddTest, "DualSlot.Policies.List.PartialAddReportsRemaining",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FListPartialAddTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeListConfig(2, false);
	UInventoryItemDefinition* Item = MakeItem(5);
	FListLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	FInventoryOpResult Result = Policy.Add(List, Item, 13, NextEntryId);
	TestEqual(TEXT("Partial status"), Result.Status, EInventoryOpStatus::Partial);
	TestEqual(TEXT("Remaining quantity correct"), Result.QuantityRemaining, 3);
	TestEqual(TEXT("Two entries filled"), List.Entries.Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListRemovePartialAndFullTest, "DualSlot.Policies.List.RemovePartialAndFull",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FListRemovePartialAndFullTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeListConfig(10);
	UInventoryItemDefinition* Item = MakeItem(10);
	FListLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	Policy.Add(List, Item, 6, NextEntryId);

	FInventoryOpResult PartialRemove = Policy.Remove(List, Item, 4);
	TestEqual(TEXT("Partial remove succeeds"), PartialRemove.Status, EInventoryOpStatus::Success);
	TestEqual(TEXT("Entry still present"), List.Entries.Num(), 1);
	TestEqual(TEXT("Remaining quantity after partial remove"), List.Entries[0].Quantity, 2);

	FInventoryOpResult FullRemove = Policy.Remove(List, Item, 2);
	TestEqual(TEXT("Full remove succeeds"), FullRemove.Status, EInventoryOpStatus::Success);
	TestEqual(TEXT("Entry deleted after full remove"), List.Entries.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListRemoveAbsentItemTest, "DualSlot.Policies.List.RemoveAbsentItemNotFound",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FListRemoveAbsentItemTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeListConfig(10);
	UInventoryItemDefinition* Item = MakeItem(10);
	FListLayoutPolicy Policy(*Config);
	FInventoryList List;

	FInventoryOpResult Result = Policy.Remove(List, Item, 1);
	TestEqual(TEXT("ItemNotFound when absent"), Result.Status, EInventoryOpStatus::ItemNotFound);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListMoveToSlotSwapTest, "DualSlot.Policies.List.MoveToSlotSwapsOccupant",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FListMoveToSlotSwapTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeListConfig(10, false);
	UInventoryItemDefinition* Item = MakeItem(10);
	FListLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	Policy.Add(List, Item, 1, NextEntryId); // entry 0 -> slot 0
	Policy.Add(List, Item, 1, NextEntryId); // entry 1 -> slot 1

	FInventoryOpResult Result = Policy.MoveToSlot(List, 0, 1);
	TestEqual(TEXT("Move succeeds"), Result.Status, EInventoryOpStatus::Success);

	FInventoryEntry* Moved = List.FindEntryById(0);
	FInventoryEntry* Occupant = List.FindEntryById(1);
	TestEqual(TEXT("Moved entry now in target slot"), Moved->SlotIndex, 1);
	TestEqual(TEXT("Occupant swapped into vacated slot"), Occupant->SlotIndex, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListMoveToSlotOutOfBoundsTest, "DualSlot.Policies.List.MoveToSlotOutOfBounds",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FListMoveToSlotOutOfBoundsTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeListConfig(4, false);
	UInventoryItemDefinition* Item = MakeItem(10);
	FListLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	Policy.Add(List, Item, 1, NextEntryId);

	FInventoryOpResult Result = Policy.MoveToSlot(List, 0, 4);
	TestEqual(TEXT("OutOfBounds when slot beyond MaxSlots"), Result.Status, EInventoryOpStatus::OutOfBounds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListMoveToCellWrongModeTest, "DualSlot.Policies.List.MoveToCellIsWrongMode",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FListMoveToCellWrongModeTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeListConfig(4);
	FListLayoutPolicy Policy(*Config);
	FInventoryList List;

	FInventoryOpResult Result = Policy.MoveToCell(List, 0, FIntPoint(0, 0), false);
	TestEqual(TEXT("WrongMode on list policy MoveToCell"), Result.Status, EInventoryOpStatus::WrongMode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FListCanAcceptBoundaryTest, "DualSlot.Policies.List.CanAcceptBoundaryWithAutoStack",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FListCanAcceptBoundaryTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* StackingConfig = MakeListConfig(2, true);
	UInventoryItemDefinition* Item = MakeItem(5);
	FListLayoutPolicy StackingPolicy(*StackingConfig);
	FInventoryList List;
	int32 NextEntryId = 0;
	StackingPolicy.Add(List, Item, 3, NextEntryId); // one stack, 2 room left, 1 free slot

	TestTrue(TEXT("Fits via stack room + free slot"), StackingPolicy.CanAccept(List, Item, 7));
	TestFalse(TEXT("Exceeds stack room + free slot capacity"), StackingPolicy.CanAccept(List, Item, 8));

	UInventoryConfig* NoStackConfig = MakeListConfig(2, false);
	FListLayoutPolicy NoStackPolicy(*NoStackConfig);
	FInventoryList NoStackList;
	int32 NoStackNextEntryId = 0;
	NoStackPolicy.Add(NoStackList, Item, 1, NoStackNextEntryId);

	TestFalse(TEXT("bAutoStack false ignores existing stack room"), NoStackPolicy.CanAccept(NoStackList, Item, 6));
	TestTrue(TEXT("Still fits as a brand new stack in the free slot"), NoStackPolicy.CanAccept(NoStackList, Item, 4));
	return true;
}

// ---------------------------------------------------------------------------
// Shared base behavior
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRemoveDrainsSmallestStacksFirstTest, "DualSlot.Policies.List.RemoveDrainsSmallestStacksFirst",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FRemoveDrainsSmallestStacksFirstTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeListConfig(10, false);
	UInventoryItemDefinition* Item = MakeItem(10);
	FListLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	Policy.Add(List, Item, 5, NextEntryId); // entry 0, qty 5
	Policy.Add(List, Item, 2, NextEntryId); // entry 1, qty 2
	Policy.Add(List, Item, 8, NextEntryId); // entry 2, qty 8

	FInventoryOpResult Result = Policy.Remove(List, Item, 2);
	TestEqual(TEXT("Remove succeeds"), Result.Status, EInventoryOpStatus::Success);
	TestEqual(TEXT("Smallest stack (entry 1) fully drained and removed"), List.Entries.Num(), 2);
	TestNull(TEXT("Entry 1 gone"), List.FindEntryById(1));
	TestNotNull(TEXT("Entry 0 untouched"), List.FindEntryById(0));
	TestEqual(TEXT("Entry 0 quantity unchanged"), List.FindEntryById(0)->Quantity, 5);
	TestEqual(TEXT("Entry 2 quantity unchanged"), List.FindEntryById(2)->Quantity, 8);
	return true;
}

// ---------------------------------------------------------------------------
// Grid mode
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridRowMajorPlacementTest, "DualSlot.Policies.Grid.OneByOneFillsRowMajor",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGridRowMajorPlacementTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeGridConfig(FIntPoint(3, 2), false);
	UInventoryItemDefinition* Item = MakeItem(1);
	FGridLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	for (int32 i = 0; i < 4; ++i)
	{
		Policy.Add(List, Item, 1, NextEntryId);
	}

	TestEqual(TEXT("Four entries placed"), List.Entries.Num(), 4);
	TestEqual(TEXT("Entry 0 at (0,0)"), List.Entries[0].TopLeft, FIntPoint(0, 0));
	TestEqual(TEXT("Entry 1 at (1,0)"), List.Entries[1].TopLeft, FIntPoint(1, 0));
	TestEqual(TEXT("Entry 2 at (2,0)"), List.Entries[2].TopLeft, FIntPoint(2, 0));
	TestEqual(TEXT("Entry 3 wraps to (0,1)"), List.Entries[3].TopLeft, FIntPoint(0, 1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridFootprintBlocksOverlapTest, "DualSlot.Policies.Grid.FootprintBlocksOverlap",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGridFootprintBlocksOverlapTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeGridConfig(FIntPoint(3, 3), false);
	UInventoryItemDefinition* BigItem = MakeItem(1, FIntPoint(2, 3));
	UInventoryItemDefinition* SmallItem = MakeItem(1, FIntPoint(1, 1));
	FGridLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	Policy.Add(List, BigItem, 1, NextEntryId); // occupies (0,0)-(1,2)
	TestEqual(TEXT("Big item placed"), List.Entries.Num(), 1);
	TestEqual(TEXT("Big item top-left"), List.Entries[0].TopLeft, FIntPoint(0, 0));

	FInventoryOpResult Result = Policy.Add(List, SmallItem, 1, NextEntryId);
	TestEqual(TEXT("Small item placed in remaining column"), Result.Status, EInventoryOpStatus::Success);
	TestEqual(TEXT("Small item placed at (2,0)"), List.Entries[1].TopLeft, FIntPoint(2, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridRotationFallbackTest, "DualSlot.Policies.Grid.RotationFallbackUsedWhenOnlyRotatedFits",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGridRotationFallbackTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	// 1x3 columns gap only; item is 3x1 (WxH), only fits rotated to 1x3.
	UInventoryConfig* Config = MakeGridConfig(FIntPoint(1, 3), false);
	UInventoryItemDefinition* Item = MakeItem(1, FIntPoint(3, 1), true);
	FGridLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	FInventoryOpResult Result = Policy.Add(List, Item, 1, NextEntryId);
	TestEqual(TEXT("Add succeeds via rotation"), Result.Status, EInventoryOpStatus::Success);
	TestEqual(TEXT("One entry placed"), List.Entries.Num(), 1);
	TestTrue(TEXT("Entry marked rotated"), List.Entries[0].bRotated);
	TestEqual(TEXT("Entry placed at origin"), List.Entries[0].TopLeft, FIntPoint(0, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridNonRotatableNeverRotatesTest, "DualSlot.Policies.Grid.NonRotatableNeverRotates",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGridNonRotatableNeverRotatesTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeGridConfig(FIntPoint(1, 3), false);
	UInventoryItemDefinition* Item = MakeItem(1, FIntPoint(3, 1), false);
	FGridLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	FInventoryOpResult Result = Policy.Add(List, Item, 1, NextEntryId);
	TestEqual(TEXT("Add fails, InventoryFull, since rotation is disallowed"), Result.Status, EInventoryOpStatus::InventoryFull);
	TestEqual(TEXT("No entries placed"), List.Entries.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridMoveToCellSuccessTest, "DualSlot.Policies.Grid.MoveToCellSuccess",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGridMoveToCellSuccessTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeGridConfig(FIntPoint(4, 4), false);
	UInventoryItemDefinition* Item = MakeItem(1);
	FGridLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	Policy.Add(List, Item, 1, NextEntryId); // entry 0 at (0,0)

	FInventoryOpResult Result = Policy.MoveToCell(List, 0, FIntPoint(2, 2), false);
	TestEqual(TEXT("Move succeeds"), Result.Status, EInventoryOpStatus::Success);
	TestEqual(TEXT("Entry moved to new cell"), List.FindEntryById(0)->TopLeft, FIntPoint(2, 2));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridMoveToCellPositionOccupiedTest, "DualSlot.Policies.Grid.MoveToCellPositionOccupied",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGridMoveToCellPositionOccupiedTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeGridConfig(FIntPoint(4, 4), false);
	UInventoryItemDefinition* Item = MakeItem(1);
	FGridLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	Policy.Add(List, Item, 1, NextEntryId); // entry 0 at (0,0)
	Policy.Add(List, Item, 1, NextEntryId); // entry 1 at (1,0)

	FInventoryOpResult Result = Policy.MoveToCell(List, 0, FIntPoint(1, 0), false);
	TestEqual(TEXT("Move blocked by occupant"), Result.Status, EInventoryOpStatus::PositionOccupied);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridMoveToCellOutOfBoundsTest, "DualSlot.Policies.Grid.MoveToCellOutOfBounds",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGridMoveToCellOutOfBoundsTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeGridConfig(FIntPoint(4, 4), false);
	UInventoryItemDefinition* Item = MakeItem(1);
	FGridLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	Policy.Add(List, Item, 1, NextEntryId); // entry 0

	FInventoryOpResult Result = Policy.MoveToCell(List, 0, FIntPoint(10, 10), false);
	TestEqual(TEXT("OutOfBounds when target exceeds grid"), Result.Status, EInventoryOpStatus::OutOfBounds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridMoveToCellInvalidRotationTest, "DualSlot.Policies.Grid.MoveToCellInvalidRequestWhenRotatingNonRotatable",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGridMoveToCellInvalidRotationTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeGridConfig(FIntPoint(4, 4), false);
	UInventoryItemDefinition* Item = MakeItem(1, FIntPoint(2, 1), false);
	FGridLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	Policy.Add(List, Item, 1, NextEntryId); // entry 0

	FInventoryOpResult Result = Policy.MoveToCell(List, 0, FIntPoint(0, 1), true);
	TestEqual(TEXT("InvalidRequest when rotating non-rotatable item"), Result.Status, EInventoryOpStatus::InvalidRequest);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridMoveToSlotWrongModeTest, "DualSlot.Policies.Grid.MoveToSlotIsWrongMode",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGridMoveToSlotWrongModeTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryConfig* Config = MakeGridConfig(FIntPoint(4, 4));
	FGridLayoutPolicy Policy(*Config);
	FInventoryList List;

	FInventoryOpResult Result = Policy.MoveToSlot(List, 0, 0);
	TestEqual(TEXT("WrongMode on grid policy MoveToSlot"), Result.Status, EInventoryOpStatus::WrongMode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGridCanAcceptFragmentedSpaceTest, "DualSlot.Policies.Grid.CanAcceptFragmentedFreeSpace",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGridCanAcceptFragmentedSpaceTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	// 4x1 grid; a 2x1 item needs a contiguous 2-cell run. Placing blockers at (1,0) and (3,0)
	// leaves free cells (0,0) and (2,0) -- two free cells by naive count, but neither adjacent
	// to the other, so no contiguous run of 2 exists and placement must fail.
	UInventoryConfig* Config = MakeGridConfig(FIntPoint(4, 1), false);
	UInventoryItemDefinition* Blocker = MakeItem(1, FIntPoint(1, 1));
	UInventoryItemDefinition* WideItem = MakeItem(1, FIntPoint(2, 1), false);
	FGridLayoutPolicy Policy(*Config);
	FInventoryList List;
	int32 NextEntryId = 0;

	FInventoryEntry EntryAt1;
	EntryAt1.EntryId = NextEntryId++;
	EntryAt1.Definition = Blocker;
	EntryAt1.Quantity = 1;
	EntryAt1.TopLeft = FIntPoint(1, 0);
	List.AddEntry(EntryAt1);

	FInventoryEntry EntryAt3;
	EntryAt3.EntryId = NextEntryId++;
	EntryAt3.Definition = Blocker;
	EntryAt3.Quantity = 1;
	EntryAt3.TopLeft = FIntPoint(3, 0);
	List.AddEntry(EntryAt3);

	// Free cells (0,0) and (2,0): count = 2, matching WideItem's footprint area, but not contiguous.
	TestFalse(TEXT("CanAccept false: free cells are non-contiguous, naive count would say yes"),
		Policy.CanAccept(List, WideItem, 1));

	FInventoryOpResult Result = Policy.Add(List, WideItem, 1, NextEntryId);
	TestEqual(TEXT("Add actually fails to place the wide item"), Result.Status, EInventoryOpStatus::InventoryFull);
	return true;
}

// ---------------------------------------------------------------------------
// ExecuteExchange (atomic crafting) — exercised through UInventoryComponent
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExchangeSuccessfulCraftTest, "DualSlot.Component.Exchange.SuccessfulCraft",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FExchangeSuccessfulCraftTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryComponent* Inv = MakeComponent(MakeListConfig(10));
	UInventoryItemDefinition* A = MakeItem(10);
	UInventoryItemDefinition* B = MakeItem(10);
	UInventoryItemDefinition* C = MakeItem(10);

	Inv->TryAddItem(A, 2);
	Inv->TryAddItem(B, 1);

	TArray<FInventoryExchangeItem> Consume{ Exchange(A, 2), Exchange(B, 1) };
	TArray<FInventoryExchangeItem> Produce{ Exchange(C, 1) };

	FInventoryOpResult Result = Inv->ExecuteExchange(Consume, Produce);
	TestEqual(TEXT("Exchange succeeds"), Result.Status, EInventoryOpStatus::Success);
	TestEqual(TEXT("A consumed"), Inv->GetItemCount(A), 0);
	TestEqual(TEXT("B consumed"), Inv->GetItemCount(B), 0);
	TestEqual(TEXT("C produced"), Inv->GetItemCount(C), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExchangeAggregatesDuplicateInputsTest, "DualSlot.Component.Exchange.AggregatesDuplicateInputs",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FExchangeAggregatesDuplicateInputsTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryComponent* Inv = MakeComponent(MakeListConfig(10));
	UInventoryItemDefinition* A = MakeItem(10);
	UInventoryItemDefinition* C = MakeItem(10);

	Inv->TryAddItem(A, 3);

	// Same definition listed twice must be summed (2+2=4 > 3 available) and fail, not double-remove.
	TArray<FInventoryExchangeItem> Consume{ Exchange(A, 2), Exchange(A, 2) };
	TArray<FInventoryExchangeItem> Produce{ Exchange(C, 1) };

	FInventoryOpResult Result = Inv->ExecuteExchange(Consume, Produce);
	TestFalse(TEXT("Aggregated demand of 4 against 3 fails"), Result.Succeeded());
	TestEqual(TEXT("A untouched by failed exchange"), Inv->GetItemCount(A), 3);
	TestEqual(TEXT("C not produced"), Inv->GetItemCount(C), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExchangeRollbackInsufficientTest, "DualSlot.Component.Exchange.RollbackOnInsufficientIngredients",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FExchangeRollbackInsufficientTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryComponent* Inv = MakeComponent(MakeListConfig(10));
	UInventoryItemDefinition* A = MakeItem(10);
	UInventoryItemDefinition* C = MakeItem(10);

	Inv->TryAddItem(A, 1);
	const TMap<int32, int32> Before = ContentCounts(Inv->GetEntries());

	TArray<FInventoryExchangeItem> Consume{ Exchange(A, 2) }; // needs 2, only 1 present
	TArray<FInventoryExchangeItem> Produce{ Exchange(C, 1) };

	FInventoryOpResult Result = Inv->ExecuteExchange(Consume, Produce);
	TestFalse(TEXT("Exchange fails"), Result.Succeeded());
	TestEqual(TEXT("A still present after rollback"), Inv->GetItemCount(A), 1);
	TestEqual(TEXT("C never produced"), Inv->GetItemCount(C), 0);
	TestTrue(TEXT("Inventory contents identical after rollback"), ContentCounts(Inv->GetEntries()).OrderIndependentCompareEqual(Before));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExchangeRollbackNoSpaceTest, "DualSlot.Component.Exchange.RollbackWhenOutputDoesNotFit",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FExchangeRollbackNoSpaceTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	// One free slot after consuming A; two distinct unstackable outputs can't both fit.
	UInventoryComponent* Inv = MakeComponent(MakeListConfig(1, /*bAutoStack*/ false));
	UInventoryItemDefinition* A = MakeItem(1);
	UInventoryItemDefinition* B = MakeItem(1);
	UInventoryItemDefinition* C = MakeItem(1);

	Inv->TryAddItem(A, 1);

	TArray<FInventoryExchangeItem> Consume{ Exchange(A, 1) };
	TArray<FInventoryExchangeItem> Produce{ Exchange(B, 1), Exchange(C, 1) }; // B fits, C has nowhere to go

	FInventoryOpResult Result = Inv->ExecuteExchange(Consume, Produce);
	TestFalse(TEXT("Exchange fails when second output can't be placed"), Result.Succeeded());
	TestEqual(TEXT("A restored after rollback"), Inv->GetItemCount(A), 1);
	TestEqual(TEXT("B not produced"), Inv->GetItemCount(B), 0);
	TestEqual(TEXT("C not produced"), Inv->GetItemCount(C), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExchangeSwapInSingleSlotTest, "DualSlot.Component.Exchange.SwapSucceedsInSingleSlot",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FExchangeSwapInSingleSlotTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	// The reason this API exists: a 1-slot inventory holding A can become B in one atomic op,
	// because the consume runs first and frees the slot the produce needs.
	UInventoryComponent* Inv = MakeComponent(MakeListConfig(1, /*bAutoStack*/ false));
	UInventoryItemDefinition* A = MakeItem(1);
	UInventoryItemDefinition* B = MakeItem(1);

	Inv->TryAddItem(A, 1);

	TArray<FInventoryExchangeItem> Consume{ Exchange(A, 1) };
	TArray<FInventoryExchangeItem> Produce{ Exchange(B, 1) };

	FInventoryOpResult Result = Inv->ExecuteExchange(Consume, Produce);
	TestEqual(TEXT("Swap succeeds"), Result.Status, EInventoryOpStatus::Success);
	TestEqual(TEXT("A gone"), Inv->GetItemCount(A), 0);
	TestEqual(TEXT("B present"), Inv->GetItemCount(B), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExchangeInvalidRequestTest, "DualSlot.Component.Exchange.InvalidRequestDoesNoWork",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FExchangeInvalidRequestTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryComponent* Inv = MakeComponent(MakeListConfig(10));
	UInventoryItemDefinition* A = MakeItem(10);
	Inv->TryAddItem(A, 2);

	// Both arrays empty.
	FInventoryOpResult Empty = Inv->ExecuteExchange({}, {});
	TestEqual(TEXT("Both empty is InvalidRequest"), Empty.Status, EInventoryOpStatus::InvalidRequest);

	// Null definition in a request.
	TArray<FInventoryExchangeItem> BadConsume{ Exchange(nullptr, 1) };
	FInventoryOpResult Null = Inv->ExecuteExchange(BadConsume, {});
	TestEqual(TEXT("Null definition is InvalidRequest"), Null.Status, EInventoryOpStatus::InvalidRequest);

	// Non-positive quantity.
	TArray<FInventoryExchangeItem> BadQty{ Exchange(A, 0) };
	FInventoryOpResult Zero = Inv->ExecuteExchange(BadQty, {});
	TestEqual(TEXT("Zero quantity is InvalidRequest"), Zero.Status, EInventoryOpStatus::InvalidRequest);

	TestEqual(TEXT("Inventory untouched by invalid requests"), Inv->GetItemCount(A), 2);
	return true;
}

// NOTE: Zero-item-events-on-rollback is asserted here via exact content equality rather than by
// counting delegate broadcasts. The events are DYNAMIC multicast delegates (AddDynamic requires a
// UFUNCTION target), and a UCLASS event-counter helper cannot be declared inside this .cpp because
// UHT generates no companion header for it. ApplyMutation derives all granular OnEntryAdded/Changed/
// Removed events by diffing the entry set before vs. after the mutator; a successful rollback restores
// Entries byte-for-byte, so that diff is empty and zero item events fire by construction — while the
// non-Success result still triggers exactly one OnOperationRejected. The state-equality assertions
// below therefore transitively verify the zero-events guarantee.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExchangeRollbackLeavesStateIdenticalTest, "DualSlot.Component.Exchange.RollbackLeavesStateIdentical",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FExchangeRollbackLeavesStateIdenticalTest::RunTest(const FString& Parameters)
{
	using namespace DualSlotTestUtil;
	UInventoryComponent* Inv = MakeComponent(MakeListConfig(10));
	UInventoryItemDefinition* A = MakeItem(10);
	UInventoryItemDefinition* B = MakeItem(10);
	UInventoryItemDefinition* C = MakeItem(10);

	Inv->TryAddItem(A, 5);
	Inv->TryAddItem(B, 3);
	const TArray<FInventoryEntry> BeforeEntries = Inv->GetEntries();
	const TMap<int32, int32> Before = ContentCounts(BeforeEntries);

	// Needs 6 A but only 5 present -> must fail and restore everything, including entry ids.
	TArray<FInventoryExchangeItem> Consume{ Exchange(A, 6) };
	TArray<FInventoryExchangeItem> Produce{ Exchange(C, 1) };

	FInventoryOpResult Result = Inv->ExecuteExchange(Consume, Produce);
	TestFalse(TEXT("Exchange fails"), Result.Succeeded());

	const TArray<FInventoryEntry> AfterEntries = Inv->GetEntries();
	TestEqual(TEXT("Same number of entries after rollback"), AfterEntries.Num(), BeforeEntries.Num());
	TestTrue(TEXT("Same entry ids and quantities after rollback"),
		ContentCounts(AfterEntries).OrderIndependentCompareEqual(Before));
	TestEqual(TEXT("A unchanged"), Inv->GetItemCount(A), 5);
	TestEqual(TEXT("B unchanged"), Inv->GetItemCount(B), 3);
	TestEqual(TEXT("C never produced"), Inv->GetItemCount(C), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
