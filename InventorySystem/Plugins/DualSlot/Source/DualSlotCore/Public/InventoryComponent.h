// Copyright Under the Bay Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryList.h"
#include "InventoryOpResult.h"
#include "InventoryComponent.generated.h"

class UInventoryConfig;
class UInventoryItemDefinition;
class FInventoryLayoutPolicy;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntryAdded, const FInventoryEntry&, Entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntryChanged, const FInventoryEntry&, Entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntryRemoved, const FInventoryEntry&, Entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOperationRejected, const FInventoryOpResult&, Result);

/**
 * One line item in an atomic exchange (crafting): a definition and how many of it.
 * Used for both the consumed inputs and the produced outputs of ExecuteExchange.
 */
USTRUCT(BlueprintType)
struct DUALSLOTCORE_API FInventoryExchangeItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UInventoryItemDefinition> Definition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ClampMin = 1))
	int32 Quantity = 1;
};

/**
 * Drop-in inventory for any Actor. Layout mode (List or Grid) comes from the
 * assigned UInventoryConfig asset - switchable at design time, no recompile.
 */
UCLASS( ClassGroup=(DualSlot), meta=(BlueprintSpawnableComponent) )
class DUALSLOTCORE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();
	virtual ~UInventoryComponent() override;

	/** Design-time configuration (layout mode, capacity, stacking rules). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryConfig> Config;

	/** Coarse "something changed, re-read GetEntries()" signal. Kept for simple UI. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnEntryAdded OnEntryAdded;

	/** Fired when an existing entry's quantity or placement changes (stacking, move). */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnEntryChanged OnEntryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnEntryRemoved OnEntryRemoved;

	/** Fired whenever an operation's Status is not Success (includes Partial). */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnOperationRejected OnOperationRejected;

	/** Adds up to Quantity of the item. Partial adds report Status=Partial with the remainder. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOpResult TryAddItem(UInventoryItemDefinition* Definition, int32 Quantity = 1);

	/** Removes up to Quantity of the item across its stacks. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOpResult RemoveItem(UInventoryItemDefinition* Definition, int32 Quantity = 1);

	/** Atomically consumes all Consume items then produces all Produce items.
	 *  Removals run first so outputs may occupy space freed by inputs.
	 *  On ANY failure the inventory is restored exactly and zero item events fire
	 *  (only OnOperationRejected, with the failing operation's result). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOpResult ExecuteExchange(const TArray<FInventoryExchangeItem>& Consume, const TArray<FInventoryExchangeItem>& Produce);

	/** Moves an entry to a linear slot (List mode only). Swaps with any occupant. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOpResult MoveEntryToSlot(int32 EntryId, int32 TargetSlot);

	/** Places an entry at a grid cell (Grid mode only). Fails if the area is occupied. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOpResult MoveEntryToCell(int32 EntryId, FIntPoint TargetCell, bool bRotated = false);

	/** True if the full quantity would fit without modifying the inventory. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CanAccept(UInventoryItemDefinition* Definition, int32 Quantity = 1) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FInventoryEntry> GetEntries() const { return InventoryList.Entries; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool FindEntry(int32 EntryId, FInventoryEntry& OutEntry) const;

	/** Total quantity of the item across all stacks. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(const UInventoryItemDefinition* Definition) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryItemDefinition* FindDefinitionById(FName ItemId) const;

protected:
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	FInventoryList InventoryList;

	int32 NextEntryId = 0;

	// TSharedPtr (not TUniquePtr): its deleter is captured where the policy is
	// created, so the UHT-generated TU never needs the complete private type.
	TSharedPtr<FInventoryLayoutPolicy> Policy;

	FInventoryLayoutPolicy* GetPolicy();
	const FInventoryLayoutPolicy* GetPolicy() const { return const_cast<UInventoryComponent*>(this)->GetPolicy(); }

	/** Runs Mutator over InventoryList and diffs entries before/after to fire granular events. */
	FInventoryOpResult ApplyMutation(TFunctionRef<FInventoryOpResult()> Mutator);
};
