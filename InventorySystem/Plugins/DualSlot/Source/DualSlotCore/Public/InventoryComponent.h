// Copyright Vicente Hotz. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItemDefinition.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FGridPosition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Row;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Column;

	FGridPosition()
		: Row(0), Column(0)
	{
	}

	FGridPosition(int32 InRow, int32 InColumn)
		: Row(InRow), Column(InColumn)
	{
	}
};

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventoryItemDefinition> Item = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGridPosition Position;

	FInventorySlot(UInventoryItemDefinition* InItem, const int32& InQuantity)
		: Item(InItem), Quantity(InQuantity)
	{
	}

	FInventorySlot() = default;

	// Important to be able to use FindByKey on TArray
	FORCEINLINE bool operator==(const FName& Key) const
	{
		return Item && Item->Id == Key;
	}

	FORCEINLINE bool operator==(const FInventorySlot& Other) const
	{
		return Item && Other.Item && Item->Id == Other.Item->Id;
	}

	FORCEINLINE bool operator==(const FGridPosition& Key) const
	{
		return Position.Column == Key.Column
			&& Position.Row == Key.Row;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(DualSlot), meta=(BlueprintSpawnableComponent) )
class DUALSLOTCORE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventorySlot> Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxSlots = 20;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory(TArray<FInventorySlot> Items);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UInventoryItemDefinition* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(UInventoryItemDefinition* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void MoveItem(UInventoryItemDefinition* Item, FGridPosition NewPosition);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryItemDefinition* GetItemById(FName ItemId);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(UInventoryItemDefinition* Item) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FInventorySlot> GetInventory();
};
