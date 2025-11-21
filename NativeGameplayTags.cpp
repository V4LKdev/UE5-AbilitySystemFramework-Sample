#include "NativeGameplayTags.h"

namespace Tags
{
	namespace Input
	{
		UE_DEFINE_GAMEPLAY_TAG(Look, "Input.Look");
		UE_DEFINE_GAMEPLAY_TAG(Move, "Input.Move");
		UE_DEFINE_GAMEPLAY_TAG(Jump, "Input.Jump");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dash, "Input.Dash", "Dash and Sprint input");
		UE_DEFINE_GAMEPLAY_TAG(Crouch, "Input.Crouch");
		UE_DEFINE_GAMEPLAY_TAG(Interact, "Input.Interact");
	}
	
	namespace State
	{
		UE_DEFINE_GAMEPLAY_TAG(Dead,		"State.Dead");
		UE_DEFINE_GAMEPLAY_TAG(Exhausted,	"State.Exhausted");
		
		UE_DEFINE_GAMEPLAY_TAG(Movement_Sprinting, "State.Movement.Sprinting");
		UE_DEFINE_GAMEPLAY_TAG(Movement_IsMoving,  "State.Movement.IsMoving");
		UE_DEFINE_GAMEPLAY_TAG(Movement_InputBlocked, "State.Movement.InputBlocked");
		
		UE_DEFINE_GAMEPLAY_TAG(AbilityInputBlocked, "State.AbilityInputBlocked");
		
		UE_DEFINE_GAMEPLAY_TAG(Health_RegenBlocked, "State.Health.RegenBlocked");
		
		UE_DEFINE_GAMEPLAY_TAG(Stamina_RegenBlocked, "State.Stamina.RegenBlocked");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stamina_RegenBlocked_ActiveAbility, "State.Stamina.RegenBlocked.ActiveAbility",
			"Stamina regeneration is blocked due to an active ability consuming stamina");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stamina_RegenBlocked_Delay, "State.Stamina.RegenBlocked.Delay",
			"Stamina regeneration is blocked due to a regen delay effect");
	}
	
	namespace Ability
	{
		UE_DEFINE_GAMEPLAY_TAG(Ability, "Ability");
		
		UE_DEFINE_GAMEPLAY_TAG(Movement_Jump, "Ability.Movement.Jump");
		UE_DEFINE_GAMEPLAY_TAG(Movement_Sprint, "Ability.Movement.Sprint");
		
		UE_DEFINE_GAMEPLAY_TAG(Stamina, "Ability.Stamina");
		UE_DEFINE_GAMEPLAY_TAG(Stamina_Drain, "Ability.Stamina.Drain");
		
		namespace Cooldown
		{
			UE_DEFINE_GAMEPLAY_TAG(Dash, "Ability.Cooldown.Dash");
		}
	}
	
	namespace Effect
	{
		UE_DEFINE_GAMEPLAY_TAG(PersistentAcrossRespawn, "Effect.PersistentAcrossRespawn");
	}
	
	namespace Data
	{
		UE_DEFINE_GAMEPLAY_TAG(Cost_Stamina, "Data.Cost.Stamina");
	}
}
