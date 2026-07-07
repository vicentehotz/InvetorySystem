// Copyright Under the Bay Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryConfig.generated.h"

/** Mutually exclusive layout modes. Selected per-inventory via UInventoryConfig, no recompile needed. */
UENUM(BlueprintType)
enum class EInventoryLayoutMode : uint8
{
	/** Linear stackable slots (classic list inventory). */
	List,
	/** Spatial grid where items occupy Width x Height cells (Resident Evil style). */
	Grid
};

/**
 * Design-time configuration for an inventory instance.
 * Assign one to a UInventoryComponent to choose its layout mode and capacity.
 */
UCLASS(BlueprintType)
class DUALSLOTCORE_API UInventoryConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	EInventoryLayoutMode LayoutMode = EInventoryLayoutMode::List;

	/** Capacity in slots. List mode only. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout",
		meta = (ClampMin = 1, EditCondition = "LayoutMode == EInventoryLayoutMode::List", EditConditionHides))
	int32 MaxSlots = 20;

	/** Grid dimensions in cells (X = columns, Y = rows). Grid mode only. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout",
		meta = (ClampMin = 1, EditCondition = "LayoutMode == EInventoryLayoutMode::Grid", EditConditionHides))
	FIntPoint GridSize = FIntPoint(10, 6);

	/** Automatically merge added items into existing non-full stacks. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	bool bAutoStack = true;
};
