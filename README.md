# Unreal Engine 5 GAS, Enhanced Input & Networking Sample

Curated technical snapshot from my in-progress fantasy action-stealth vertical slice, focused on scalable, networked gameplay systems built with Unreal Engine 5 and the Gameplay Ability System (GAS).

This repo is a **tech sample** (not the full game). Canonical development lives in Perforce; GitHub hosts a clean, public snapshot of the core architecture.

- **Engine:** Unreal Engine 5 (C++)
- **Core concepts:**
  - **Gameplay Ability System (GAS)** for abilities, stats, effects, and state.
  - **Enhanced Input → GAS input routing** with possession-safe bindings.
  - Custom Character Movement Component + GAS for predicted, networked movement.
  - Data-driven tuning for abilities/characters.
  - Event-driven architecture for decoupled gameplay systems.

---

## Architecture Highlights

### Gameplay Ability System (`AbilitySystem/`)
- `UIsekaiAbilitySystemComponent`: custom ASC hub for granting/activating abilities and routing tags/events.
- `UIsekaiAttributeSet`: replicated attributes (`Health`, `Stamina`, etc.) with clamping and change notifications.
- `UIsekaiGameplayAbility`: base class with shared cost/cooldown/activation patterns.
- `UIsekaiAbilitySet`: data asset bundling abilities, effects, and attributes into reusable loadouts.

### ASC Ownership
A key decision is where the `AbilitySystemComponent` lives. This project uses a hybrid approach for multiplayer correctness:

- **Players:** ASC owned by `IsekaiPlayerState` (persists across respawns, exists on server + clients, ideal identity/ability owner).
- **AI:** ASC owned by the `AIsekaiCharacterBase` pawn (simpler and sufficient for NPC lifetimes).

### GAS-Driven Character Movement (`Character/`)
A key focus is integrating GAS with a custom movement component derived from `UCharacterMovementComponent` (CMC) to support responsive, predicted movement abilities in multiplayer.

- **Challenge:** Naively modifying CMC state from abilities can cause prediction/jitter because CMC owns client-side movement prediction.
- **Solution:** `UIsekaiCharacterMovementComponent` works in tandem with GAS:
  - CMC owns movement speeds/states (`BaseWalkSpeed`, `SprintSpeedMultiplier`, `bIsExhausted`).
  - Abilities (e.g., `UGA_Sprint`) handle activation logic, input intent, and stamina costs.
  - Abilities signal **intent** (e.g., `StartSprinting()`), while CMC applies authoritative changes via `GetMaxSpeed()`/internal flags.
  - State sync is bridged through Gameplay Tags (e.g., `State.Exhausted`) so GAS and CMC remain decoupled while still prediction-safe.

### Enhanced Input → GAS Input Routing
Input is data-driven and routed into GAS using tagged actions and a small Enhanced Input wrapper.

- **Data-driven bindings:** Inputs are defined as `FTaggedInputAction` entries inside `UInputConfig` (InputAction + GameplayTag).
- **Custom input component:** `UIsekaiInputComponent` extends `UEnhancedInputComponent` and provides helpers to:
  - Bind all ability actions from `InputConfig` (Started → Pressed, Completed/Canceled → Released).
  - Bind native actions (Move/Look) by tag.
- **Controller-level setup:** `AIsekaiPlayerController::SetupInputComponent`:
  - Adds the mapping context from `InputConfig`.
  - Binds ability input tags to `Input_AbilityTagPressed/Released`.
- **ASC-side input processing:** Pressed/Released tags are queued on the ASC via:
  - `AbilityInputTagPressed(Tag)` / `AbilityInputTagReleased(Tag)`
  - The ASC resolves **Pressed / Held / Released** each frame in `ProcessAbilityInput`, activating abilities based on each ability’s activation policy (`OnInputTriggered`, `WhileInputActive`) and forwarding replicated input events to active abilities.

This keeps input mappings in data, bindings possession-safe, and ability activation centralized in GAS.

### UI Bridge (`Widget/`)
To keep UI decoupled from GAS, the project uses a `UIsekaiUIBridge` component on `AIsekaiCharacterBase`.

- Subscribes to ASC attribute/tag changes and re-emits minimal delegates (e.g., `OnHealthChanged`, `OnStaminaChanged`).
- Widgets bind to the bridge (not the ASC), avoiding polling and keeping HUD resilient to GAS refactors.

### Data-Driven Design
Gameplay tuning lives in data assets and Gameplay Effects, enabling fast iteration in editor without touching C++.

---

## Networking
Multiplayer is server-authoritative with GAS-driven replication/prediction and tag-based state sync:

- Abilities execute on server; effects/state replicate to clients.
- Movement abilities leverage CMC prediction for responsive feel under latency.
- Attributes replicate efficiently to keep clients consistent with the authoritative game state.
