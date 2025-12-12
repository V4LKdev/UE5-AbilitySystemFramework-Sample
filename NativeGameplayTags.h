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
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttackPrimary);
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
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Crouching);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Health_RegenBlocked);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina_RegenBlocked);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina_RegenBlocked_ActiveAbility);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina_RegenBlocked_Delay);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Visibility_Camouflaged);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Visibility_Invisible);
	}

	// Ability identity/family/cooldowns
	namespace Ability
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Jump);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Sprint);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Crouch);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stamina_Drain);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Combat_MeleeAttack);
		
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
	
	namespace Noise
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Footstep);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Footstep_Walk);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Footstep_Run);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Footstep_Crouch);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Jump);
	}

	namespace SquadMessage
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(EnemySpotted);
	}
	
	namespace Event
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Montage_Hit);
	}
	
}
