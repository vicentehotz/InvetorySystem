// Copyright Under the Bay Studios. All Rights Reserved.

#include "InventoryList.h"
#include "InventoryItemDefinition.h"

FIntPoint FInventoryEntry::GetFootprint() const
{
	if (!Definition)
	{
		return FIntPoint(1, 1);
	}
	return bRotated ? FIntPoint(Definition->Size.Y, Definition->Size.X) : Definition->Size;
}
