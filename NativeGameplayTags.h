#pragma once

#include "Runtime/GameplayTags/Public/NativeGameplayTags.h"

namespace Tags
{
	namespace Input
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Look);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Move);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Jump);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dash);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crouch);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interact);
	}

	// Character states
	namespace State
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dead);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Exhausted);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(AbilityInputBlocked);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Sprinting);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_IsMoving);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_InputBlocked);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Health_RegenBlocked);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina_RegenBlocked);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina_RegenBlocked_ActiveAbility);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina_RegenBlocked_Delay);
	}

	// Ability identity/family/cooldowns
	namespace Ability
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Jump);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Sprint);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina_Drain);
		
		namespace Cooldown
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dash);
		}
	}
	
	// Effect types/lifecycle policy, etc.
	namespace Effect
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(PersistentAcrossRespawn);
	}

	// set-by-caller / costs / numeric metadata
	namespace Data
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cost_Stamina);
	}
}
