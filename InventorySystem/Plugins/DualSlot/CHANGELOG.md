# Changelog

All notable changes to the DualSlot plugin are documented here.
This project follows [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added
- `UInventoryComponent::ExecuteExchange(Consume[], Produce[])` — atomic multi-item
  exchange for crafting. Removes all inputs first (so outputs can reuse freed space,
  enabling in-place swaps in a full inventory) then adds all outputs; on any failure
  the inventory is restored exactly and no entry events fire (only `OnOperationRejected`).
  New `FInventoryExchangeItem` `{ Definition, Quantity }` struct describes each line item.
- Automation tests covering successful craft, input aggregation, rollback on insufficient
  ingredients, rollback on no output space, the single-slot swap case, invalid requests,
  and state-identity after rollback.

## [1.0.0] — Initial release

### Added
- `UInventoryComponent` — a drop-in inventory for any Actor, no base class required.
- Design-time layout selection via `UInventoryConfig` Data Asset:
  - **List** mode — linear, stackable slots with configurable capacity.
  - **Grid** mode — spatial placement with per-item footprints, 90° rotation,
    collision, and row-major first-fit auto-placement.
- `UInventoryItemDefinition` item assets with stacking, grid size, rotation flag,
  icon, and a composition-based `UInventoryItemFragment` extension system.
- Public Blueprint API: `TryAddItem`, `RemoveItem`, `MoveEntryToSlot`,
  `MoveEntryToCell`, `CanAccept`, `GetEntries`, `FindEntry`, `GetItemCount`,
  returning a structured `FInventoryOpResult` (Success / Partial / rejection reasons).
- Granular events: `OnEntryAdded`, `OnEntryChanged`, `OnEntryRemoved`,
  `OnOperationRejected`, plus a coarse `OnInventoryUpdated`.
- `IInventoryOwnerInterface` for cast-free inventory discovery on any Actor.
- `UInventoryWidget` base widget forwarding events to Blueprint hooks.
- 21 automation tests covering List and Grid placement rules.
- Replication-ready data layer (`FFastArraySerializer`); networking RPCs are
  planned for a future release.
