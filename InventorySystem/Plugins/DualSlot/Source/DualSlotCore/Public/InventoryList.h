// Copyright Vicente Hotz. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryList.generated.h"

class UInventoryItemDefinition;

/**
 * One runtime stack of items. Placement fields are interpreted by the active layout mode:
 *  - List: SlotIndex is the linear slot; TopLeft/bRotated unused.
 *  - Grid: TopLeft is the top-left cell, bRotated swaps the footprint; SlotIndex unused.
 */
USTRUCT(BlueprintType)
struct DUALSLOTCORE_API FInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryItemDefinition> Definition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIntPoint TopLeft = FIntPoint(INDEX_NONE, INDEX_NONE);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bRotated = false;

	/** Effective footprint in cells, accounting for rotation. */
	FIntPoint GetFootprint() const;

	bool IsValid() const { return Definition != nullptr && Quantity > 0; }
};

/**
 * Replication-ready container for inventory entries (FastArray delta serialization).
 * Mutations must go through the owning UInventoryComponent so dirty-marking stays correct.
 */
USTRUCT()
struct DUALSLOTCORE_API FInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FInventoryEntry> Entries;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryEntry, FInventoryList>(Entries, DeltaParms, *this);
	}

	FInventoryEntry& AddEntry(const FInventoryEntry& Entry)
	{
		FInventoryEntry& Added = Entries.Add_GetRef(Entry);
		MarkItemDirty(Added);
		return Added;
	}

	void RemoveEntryAt(int32 Index)
	{
		Entries.RemoveAt(Index);
		MarkArrayDirty();
	}

	void MarkEntryDirty(FInventoryEntry& Entry)
	{
		MarkItemDirty(Entry);
	}
};

template <>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};
