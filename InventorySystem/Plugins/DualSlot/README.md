# DualSlot

**List & Grid inventory, one component.** Choose the layout mode per inventory at design time — no recompiling, no subclassing.

DualSlot ships as two runtime modules:
- `DualSlotCore` — data, logic, replication container. No UMG dependency.
- `DualSlotUI` — a base widget you can subclass in Blueprint to render either layout.

---

## Quick start

1. Add a `UInventoryComponent` to any Actor (C++ or Blueprint — no base class required).
2. Create a `UInventoryConfig` Data Asset (Content Browser → *Miscellaneous → Data Asset → InventoryConfig*), set `LayoutMode` to **List** or **Grid**, and assign it to the component's `Config` property.
3. Create one or more `UInventoryItemDefinition` Data Assets to describe your items.
4. Call `TryAddItem` / `RemoveItem` from gameplay code or Blueprint.

That's it — switching an inventory between List and Grid is swapping which `UInventoryConfig` asset is assigned, not writing new code.

---

## Configuring an inventory: `UInventoryConfig`

| Property | Applies to | Meaning |
|---|---|---|
| `LayoutMode` | both | `List` (linear stackable slots) or `Grid` (spatial, Tetris/Resident Evil style) |
| `MaxSlots` | List | number of slots |
| `GridSize` | Grid | grid dimensions in cells (X = columns, Y = rows) |
| `bAutoStack` | both | merge added items into existing non-full stacks before opening new ones |

If a `UInventoryComponent` has no `Config` assigned, it falls back to a transient List config (20 slots, auto-stack on) and logs a warning — it will never crash, but always assign a `Config` in real usage.

---

## Defining items: `UInventoryItemDefinition`

A `UInventoryItemDefinition` is a `UPrimaryDataAsset` describing an item type. Create one per item in the Content Browser.

| Property | Meaning |
|---|---|
| `Id` | stable identifier (`FName`) |
| `Name` | display text |
| `Icon` | soft reference to a `UTexture2D` |
| `MaxStackSize` | max quantity per stack |
| `Size` | footprint in grid cells (`X`, `Y`); ignored in List mode (treated as 1x1) |
| `bRotatable` | whether the item may be placed rotated 90° in Grid mode |
| `StoredItem` | optional link to an `IStorableItemInterface` world representation |
| `Fragments` | extension point — see below |

### Fragments — extending items without touching the plugin

`UInventoryItemFragment` is an abstract, instanced `UObject` you subclass (in C++ or Blueprint) to attach arbitrary data to an item — consumable stats, equip slot, crafting recipe, whatever your game needs — without modifying `UInventoryItemDefinition` itself.

```cpp
UCLASS()
class UConsumableFragment : public UInventoryItemFragment
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) float HealAmount = 20.f;
};
```

Retrieve it from gameplay code:

```cpp
if (UConsumableFragment* Consumable = ItemDef->FindFragmentByClass<UConsumableFragment>())
{
    Heal(Consumable->HealAmount);
}
```

(`FindFragmentByClass` is also `BlueprintCallable`.)

---

## Public API — `UInventoryComponent`

All functions are `BlueprintCallable`.

| Function | Returns | Notes |
|---|---|---|
| `TryAddItem(Definition, Quantity)` | `FInventoryOpResult` | stacks first (if `bAutoStack`), then opens new entries |
| `RemoveItem(Definition, Quantity)` | `FInventoryOpResult` | drains smallest stacks first |
| `ExecuteExchange(Consume[], Produce[])` | `FInventoryOpResult` | **atomic** crafting swap — see below |
| `MoveEntryToSlot(EntryId, TargetSlot)` | `FInventoryOpResult` | **List mode only**; swaps with any occupant |
| `MoveEntryToCell(EntryId, TargetCell, bRotated)` | `FInventoryOpResult` | **Grid mode only**; fails if the area is occupied |
| `CanAccept(Definition, Quantity)` | `bool` | true if the full quantity would fit without mutating anything |
| `GetEntries()` | `TArray<FInventoryEntry>` | current contents |
| `FindEntry(EntryId, OutEntry)` | `bool` | look up a single entry by its stable handle |
| `GetItemCount(Definition)` | `int32` | total quantity across all stacks |
| `FindDefinitionById(ItemId)` | `UInventoryItemDefinition*` | |

Calling `MoveEntryToSlot` in Grid mode (or `MoveEntryToCell` in List mode) returns `Status = WrongMode` rather than doing nothing silently.

### Atomic crafting: `ExecuteExchange`

```cpp
FInventoryOpResult ExecuteExchange(
    const TArray<FInventoryExchangeItem>& Consume,
    const TArray<FInventoryExchangeItem>& Produce);
```

Consumes **all** `Consume` inputs and produces **all** `Produce` outputs as a single all-or-nothing operation. Removals run first, so an output can occupy a slot freed by an input — a 1-slot inventory holding item *A* can be crafted into item *B* even though *A* and *B* never coexist. `FInventoryExchangeItem` is simply a `{ Definition, Quantity }` pair.

If **any** step fails (not enough ingredients, or no room for an output — a `Partial` result counts as failure here, since the operation is atomic), the inventory is restored **exactly** and no `OnEntryAdded/Changed/Removed` events fire; only a single `OnOperationRejected` is broadcast carrying the failing step's result. On success the net difference is broadcast as normal granular events.

Rules:
- Duplicate definitions within `Consume` are summed before removal.
- Empty `Consume` (pure production) or empty `Produce` (pure consumption) is allowed; **both** empty, or any entry with a null definition or quantity ≤ 0, returns `InvalidRequest` and does no work.

```cpp
// 2×Scrap + 1×Cloth  ->  1×Bandage
TArray<FInventoryExchangeItem> In  { {Scrap, 2}, {Cloth, 1} };
TArray<FInventoryExchangeItem> Out { {Bandage, 1} };
if (Inventory->ExecuteExchange(In, Out).Succeeded()) { /* crafted */ }
```

### `FInventoryOpResult`

```cpp
struct FInventoryOpResult
{
    EInventoryOpStatus Status;      // Success, Partial, InventoryFull, ItemNotFound,
                                     // PositionOccupied, OutOfBounds, WrongMode, InvalidRequest
    int32 QuantityRemaining;        // non-zero only when Status == Partial
};
```

Always check `Status` — a request for more items than fit returns `Partial` with however many didn't make it into `QuantityRemaining`, not a hard failure.

### `FInventoryEntry`

The runtime record for one stack. `EntryId` is a stable handle — safe to hold onto across frames and to pass to `MoveEntryToSlot`/`MoveEntryToCell`/`FindEntry`. `SlotIndex` is meaningful in List mode; `TopLeft`/`bRotated` are meaningful in Grid mode; the unused set is left at defaults.

---

## Events

| Delegate | Fires when |
|---|---|
| `OnInventoryUpdated` | any successful or partial mutation — coarse "re-read `GetEntries()`" signal, good enough for a simple full-refresh UI |
| `OnEntryAdded(Entry)` | a new entry appears |
| `OnEntryChanged(Entry)` | an existing entry's quantity or placement changes |
| `OnEntryRemoved(Entry)` | an entry is fully removed |
| `OnOperationRejected(Result)` | any operation whose `Status != Success` (includes `Partial`) |

The granular events are computed by diffing the inventory before/after each mutation, so a single call that touches several stacks (e.g. an add that fills two existing stacks and opens a new one) fires one `Changed`/`Changed`/`Added` correctly instead of one opaque refresh.

---

## Building UI: `UInventoryWidget` and `IInventoryOwnerInterface`

`DualSlotUI`'s `UInventoryWidget` is a base `UUserWidget`:

1. Call `InitializeWithInventory(InventoryComponent)` once you have a reference to the component.
2. Subclass in Blueprint and implement `OnEntryAddedBP` / `OnEntryChangedBP` / `OnEntryRemovedBP` / `OnOperationRejectedBP` to render your own List or Grid view incrementally.
3. Alternatively, override `OnInventoryUpdated` for a simple full-rebuild UI (the default C++ implementation does exactly that against a `UUniformGridPanel` bound as `InventoryGrid`, placing each entry at its real `TopLeft` cell in Grid mode).

To find the inventory without hard-coding a cast to a concrete Actor class, have the owning Actor implement `IInventoryOwnerInterface`:

```cpp
// In your Actor / Character
UFUNCTION(BlueprintNativeEvent)
UInventoryComponent* GetInventoryComponent() const;
```

Any other system (pickups, UI, AI) can then do:

```cpp
if (OtherActor->Implements<UInventoryOwnerInterface>())
{
    UInventoryComponent* Inv = IInventoryOwnerInterface::Execute_GetInventoryComponent(OtherActor);
}
```

This is how `TP_PickUpComponent` in the sample project decides whether an overlapping Actor can receive an item — no cast to a specific Character class required.

---

## Multiplayer readiness

DualSlot v1 ships **replication-ready but without networking wired up**. `FInventoryEntry`/`FInventoryList` are already built on `FFastArraySerializer`, so a future release can add `GetLifetimeReplicatedProps` and server-authoritative RPCs without changing the data format. As of this version:

- There is no `HasAuthority()` gating and no client→server RPCs.
- All mutation functions run wherever they're called; wrap them yourself if you need authority checks today.
- Do not rely on this container replicating until a future release adds the network layer.

---

## Known limitations / roadmap

- Replication RPCs and authority checks: planned, not implemented (see above).
- No built-in drag-and-drop widget implementation — `UInventoryWidget` gives you the data and events, the visuals are yours.
- The sample FPS project ships C++-side integration (`TP_PickUpComponent`, the two `UInventoryConfig` assets) but the Blueprint-side wiring (`BP_PickUp_Rifle`'s cast node, `BP_FirstPersonCharacter` implementing `IInventoryOwnerInterface` and holding the component) is a manual follow-up step, not yet done in this repository.
