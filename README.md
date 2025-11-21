# Unreal Engine 5 GAS, Enhanced Input & Networking Sample

Curated technical snapshot from my in-progress fantasy action-stealth vertical slice, focused on scalable, networked gameplay systems built with Unreal Engine 5 and the Gameplay Ability System (GAS).

This repo is intentionally a **tech sample** (not the full game). Canonical development lives in Perforce; GitHub hosts a clean, public snapshot of the core architecture.

- **Engine:** Unreal Engine 5 (C++)
- **Core concepts:**
  - **Gameplay Ability System (GAS)** for abilities, stats, effects, and state.
  - **Enhanced Input → GAS activation** and possession-safe input routing.
  - Custom Character Movement Component + GAS for predicted, networked movement.
  - Data-driven tuning for abilities/characters.
  - Event-driven architecture for decoupled gameplay systems.

---

## Architecture Highlights

### GAS-Driven Character Movement (`/Character/`)
A key focus is integrating GAS with `UCharacterMovementComponent` (CMC) to support responsive, predicted movement abilities in multiplayer.

- **Challenge:** Naively modifying CMC state from abilities can cause prediction/jitter issues because CMC owns client-side movement prediction.
- **Solution:** A custom `UIsekaiCharacterMovementComponent` works in tandem with GAS:
  - CMC owns movement speeds/states (`BaseWalkSpeed`, `SprintSpeedMultiplier`, `bIsExhausted`).
  - Abilities (e.g., `UGA_Sprint`) own activation logic, input handling, and stamina costs.
  - Abilities signal **intent** (e.g., `StartSprinting()`), while CMC applies authoritative speed/state changes via `GetMaxSpeed()`.
  - State is bridged via Gameplay Tags (e.g., `State.Exhausted`) so GAS and CMC stay decoupled.

### Gameplay Ability System (`/AbilitySystem/`)
- `UIsekaiAbilitySystemComponent`: custom ASC hub.
- `UIsekaiAttributeSet`: replicated attributes (`Health`, `Stamina`, etc.) with clamping.
- `UIsekaiGameplayAbility`: shared base for costs/cooldowns/activation.
- `UIsekaiAbilitySet`: data asset bundling abilities/effects/attributes for easy loadouts.

### Data-Driven Design
Gameplay tuning lives in data assets and Gameplay Effects, enabling iteration without touching C++.

---

## Networking
Multiplayer is server-authoritative with GAS-driven replication/prediction:
- Abilities execute on server; effects/state replicate to clients.
- Movement abilities leverage CMC prediction for responsive feel under latency.
- Attributes replicate efficiently for consistent game state.
