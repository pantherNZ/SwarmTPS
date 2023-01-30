// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Utility.h"
#include "SkillSystem.generated.h"

USTRUCT(BlueprintType)
struct FPassiveSkills
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades") float MovementSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades") float HealthRegen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades") float ReviveSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades") float JumpHeight;
};

UCLASS()
class SWARM_API ASkillSystem : public AActor
{
	GENERATED_BODY()

		// Functions
public:
	ASkillSystem();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void Initialise();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool SkillSlotFull();

	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	int32 GetCurrentUpgradePercent(int32 Stat);

	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	float GetUpgradeStepValue(int32 Stat);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetSkill(ESkillType SkillType, FString Name);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void UpgradePassive(ESkillType SkillType);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ExecuteSkill();

	// Members
public:
	//UPROPERTY(BlueprintReadWrite, Category = "Stats") ASwarmCharacter* OwningPlayer;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) ESkillType CurrentSkill;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") FString CurrentSkillName;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) FPassiveSkills PassiveSkills;
	UPROPERTY(BlueprintReadWrite, Category = "Stats", Replicated) TArray<FPassiveSkills> PassiveSkillsMinMax;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") UClass *Mines;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") int32 NumMines;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") UParticleSystem* TeleportPlaceParticle;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") UParticleSystem* TeleportParticle;
	UPROPERTY(BlueprintReadWrite, Category = "Stats") UParticleSystem* MedkitParticle;
	FVector TeleportPosition;
};