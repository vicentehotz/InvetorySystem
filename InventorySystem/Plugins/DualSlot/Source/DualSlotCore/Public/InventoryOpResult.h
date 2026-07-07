// Copyright Under the Bay Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryOpResult.generated.h"

UENUM(BlueprintType)
enum class EInventoryOpStatus : uint8
{
	Success,
	/** Only part of the requested quantity was processed (see QuantityRemaining). */
	Partial,
	InventoryFull,
	ItemNotFound,
	PositionOccupied,
	OutOfBounds,
	/** Operation not valid for the active layout mode (e.g. MoveEntryToCell in List mode). */
	WrongMode,
	InvalidRequest
};

USTRUCT(BlueprintType)
struct DUALSLOTCORE_API FInventoryOpResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	EInventoryOpStatus Status = EInventoryOpStatus::InvalidRequest;

	/** Quantity that could NOT be processed (0 on full success). */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 QuantityRemaining = 0;

	bool Succeeded() const { return Status == EInventoryOpStatus::Success; }

	static FInventoryOpResult Ok() { return { EInventoryOpStatus::Success, 0 }; }
	static FInventoryOpResult Fail(EInventoryOpStatus InStatus, int32 Remaining = 0) { return { InStatus, Remaining }; }
};
