// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemData.h"
#include "InventoryWidget.h"
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
	UItemData* Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGridPosition Position;

	FInventorySlot(UItemData* Item, const int32& Quantity)
		: Item(Item), Quantity(Quantity)
	{
	}

	FInventorySlot() = default;

	// Important to be able to use FindByKey on TArray
	FORCEINLINE bool operator==(const FName& Key) const
	{
		return Item->Id == Key;
	}

	FORCEINLINE bool operator==(const FInventorySlot& Other) const
	{
		return Item->Id == Other.Item->Id;
	}

	FORCEINLINE bool operator==(const FGridPosition& Key) const
	{
		return Position.Column == Key.Column 
			&& Position.Row == Key.Row;
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EASYINVENTORYSYSTEM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventorySlot> Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UInventoryWidget* Widget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxSlots = 20;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory(TArray<FInventorySlot> Items);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UItemData* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(UItemData* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void MoveItem(UItemData* Item, FGridPosition NewPosition);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemData* GetItemById(FName ItemId);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(UItemData* Item) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FInventorySlot> GetInventory();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
