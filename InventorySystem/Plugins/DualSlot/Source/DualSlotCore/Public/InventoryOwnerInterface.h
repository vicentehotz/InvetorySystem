// Copyright Vicente Hotz. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryOwnerInterface.generated.h"

class UInventoryComponent;

/**
 * Implement on any Actor that exposes an inventory, so callers never need to
 * know or cast to a concrete Actor class to find it.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UInventoryOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class DUALSLOTCORE_API IInventoryOwnerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	UInventoryComponent* GetInventoryComponent() const;
};
