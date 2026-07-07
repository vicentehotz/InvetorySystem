// Copyright Under the Bay Studios. All Rights Reserved.

#pragma once

#include "Modes/InventoryLayoutPolicy.h"

/** Spatial grid: entries occupy Width x Height cells, optionally rotated 90 degrees. */
class FGridLayoutPolicy final : public FInventoryLayoutPolicy
{
public:
	using FInventoryLayoutPolicy::FInventoryLayoutPolicy;

	virtual FInventoryOpResult MoveToCell(FInventoryList& List, int32 EntryId, FIntPoint TargetCell, bool bRotated) override;
	virtual bool CanAccept(const FInventoryList& List, const UInventoryItemDefinition* Definition, int32 Quantity) const override;

protected:
	virtual bool FindFreePlacement(const FInventoryList& List, const UInventoryItemDefinition* Definition, FInventoryEntry& OutPlacement) const override;

private:
	/** True if a footprint at TopLeft fits in bounds and overlaps no entry except IgnoreEntryId. */
	bool IsAreaFree(const FInventoryList& List, FIntPoint TopLeft, FIntPoint Footprint, int32 IgnoreEntryId = INDEX_NONE) const;

	static bool Overlaps(FIntPoint TopLeftA, FIntPoint FootprintA, FIntPoint TopLeftB, FIntPoint FootprintB);
};
