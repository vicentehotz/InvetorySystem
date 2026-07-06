// Copyright Vicente Hotz. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryList.h"
#include "InventoryOpResult.h"

class UInventoryConfig;
class UInventoryItemDefinition;

/**
 * Internal strategy for layout-specific placement rules. Never exposed outside the module;
 * the layout mode is data (UInventoryConfig), not a class the consumer deals with.
 */
class FInventoryLayoutPolicy
{
public:
	explicit FInventoryLayoutPolicy(const UInventoryConfig& InConfig) : Config(InConfig) {}
	virtual ~FInventoryLayoutPolicy() = default;

	/** Adds quantity, stacking first when allowed, then placing new entries. Partial adds permitted. */
	virtual FInventoryOpResult Add(FInventoryList& List, UInventoryItemDefinition* Definition, int32 Quantity, int32& NextEntryId);

	/** Removes quantity across stacks of the definition. Mode-agnostic. */
	FInventoryOpResult Remove(FInventoryList& List, UInventoryItemDefinition* Definition, int32 Quantity);

	/** List-mode placement. Default: wrong mode. */
	virtual FInventoryOpResult MoveToSlot(FInventoryList& List, int32 EntryId, int32 TargetSlot)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::WrongMode);
	}

	/** Grid-mode placement. Default: wrong mode. */
	virtual FInventoryOpResult MoveToCell(FInventoryList& List, int32 EntryId, FIntPoint TargetCell, bool bRotated)
	{
		return FInventoryOpResult::Fail(EInventoryOpStatus::WrongMode);
	}

	/** True if the full quantity could be added without mutating the list. */
	virtual bool CanAccept(const FInventoryList& List, const UInventoryItemDefinition* Definition, int32 Quantity) const = 0;

protected:
	const UInventoryConfig& Config;

	/** Quantity absorbable by existing non-full stacks of the definition. */
	int32 StackableRoom(const FInventoryList& List, const UInventoryItemDefinition* Definition) const;

	/** Fills existing stacks; returns quantity still left to place. */
	int32 FillExistingStacks(FInventoryList& List, UInventoryItemDefinition* Definition, int32 Quantity);

	/** Finds a free position for a new entry of the definition; returns false if none. */
	virtual bool FindFreePlacement(const FInventoryList& List, const UInventoryItemDefinition* Definition, FInventoryEntry& OutPlacement) const = 0;
};
