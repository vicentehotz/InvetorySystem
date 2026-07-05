// Copyright Vicente Hotz. All Rights Reserved.

#pragma once

#include "Modes/InventoryLayoutPolicy.h"

/** Linear stackable slots. Placement = SlotIndex in [0, MaxSlots). */
class FListLayoutPolicy final : public FInventoryLayoutPolicy
{
public:
	using FInventoryLayoutPolicy::FInventoryLayoutPolicy;

	virtual FInventoryOpResult MoveToSlot(FInventoryList& List, int32 EntryId, int32 TargetSlot) override;
	virtual bool CanAccept(const FInventoryList& List, const UInventoryItemDefinition* Definition, int32 Quantity) const override;

protected:
	virtual bool FindFreePlacement(const FInventoryList& List, const UInventoryItemDefinition* Definition, FInventoryEntry& OutPlacement) const override;

private:
	int32 FindFreeSlot(const FInventoryList& List) const;
	int32 CountFreeSlots(const FInventoryList& List) const;
};
