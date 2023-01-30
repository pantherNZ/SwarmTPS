#pragma once

#include "Engine.h"
#include "SwarmCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EGT_PISTOL UMETA(DisplayName = "Pistol"),
	EGT_GRENADELAUNCHER UMETA(DisplayName = "Grenade Launcher"),
	EGT_KATANA UMETA(DisplayName = "Katana"),
	EGT_RIFLE UMETA(DisplayName = "Assault Rifle"),
	EGT_FLAMETHROWER UMETA(DisplayName = "Flamethrower"),
	EGT_NONE UMETA(DisplayName = "NONE"),
};

UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	EMS_CHASING UMETA(DisplayName = "Chasing"),
	EMS_ATTACKING UMETA(DisplayName = "Attacking"),
	EMS_CHARGING UMETA(DisplayName = "Charging"),
	EMS_FLEE UMETA(DisplayName = "Fleeing"),
	EMS_DEAD UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EMonsterType : uint8
{
	EMT_SWARMER_SMALL UMETA(DisplayName = "Small Swamer"),
	EMT_SWARMER_MEDIUM UMETA(DisplayName = "Medium Swamer"),
	EMT_SWARMER_LARGE UMETA(DisplayName = "Large Swamer"),
	EMT_SLAMMER UMETA(DisplayName = "Slammer"),
	EMT_SPITTER UMETA(DisplayName = "Spitter"),
	EMT_BROODMOTHER UMETA(DisplayName = "Broodmother")
};

UENUM(BlueprintType)
enum class EGameState : uint8
{
	EGS_MENU UMETA(DisplayName = "In Menu"),
	EGS_WAITINGFORPLAYERS UMETA(DisplayName = "Waiting For Players"),
	EGS_PLAYERSNOTREADY UMETA(DisplayName = "Players Not Ready"),
	EGS_WAVEINTERVAL UMETA(DisplayName = "Wave Interval"),
	EGS_WAVEACTIVE UMETA(DisplayName = "Wave Active"),
	EGS_GAMEOVER UMETA(DisplayName = "Game Over"),
};

UENUM(BlueprintType)
enum class EAIState : uint8
{
	EAIS_WANDERING UMETA(DisplayName = "Wandering"),
	EAIS_CHASING UMETA(DisplayName = "Chasing Target"),
	EAIS_ATTACKING UMETA(DisplayName = "Attacking Target"),
	EAIS_FOLLOWING UMETA(DisplayName = "Following Other Monster"),
};

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	EST_NONE UMETA(DisplayName = "Empty"),
	EST_TELEPORT_PLACE UMETA(DisplayName = "Teleport Placement"),
	EST_TELEPORT UMETA(DisplayName = "Teleport"),
	EST_MOVEMENTSPEED UMETA(DisplayName = "Movement Speed"),
	EST_HEALTHREGEN UMETA(DisplayName = "Health Regeneration"),
	EST_REVIVESPEED UMETA(DisplayName = "Revive Speed"),
	EST_JUMPHEIGHT UMETA(DisplayName = "Jump Height"),
	EST_SELFREVIVE UMETA(DisplayName = "Self Revive"),
	EST_MINEPLACEMENT UMETA(DisplayName = "Mines"),
	EST_MEDKIT UMETA(DisplayName = "Medkit")
};

static FORCEINLINE bool VTraceSphere(
	AActor* ActorToIgnore,
	const FVector& Start,
	const FVector& End,
	const float Radius,
	FHitResult& HitOut,
	ECollisionChannel TraceChannel = ECC_Pawn
	)
{
	FCollisionQueryParams TraceParams(FName(TEXT("VictoreCore Trace")), true, ActorToIgnore);
	TraceParams.bTraceComplex = true;
	//TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;

	//Ignore Actors
	TraceParams.AddIgnoredActor(ActorToIgnore);

	//Re-initialize hit info
	HitOut = FHitResult(ForceInit);

	//Get World Source
	TObjectIterator< APlayerController > ThePC;
	if (!ThePC) return false;

	return ThePC->GetWorld()->SweepSingleByChannel(
		HitOut,
		Start,
		End,
		FQuat(),
		TraceChannel,
		FCollisionShape::MakeSphere(Radius),
		TraceParams
		);
}