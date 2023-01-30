// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Utility.h"
#include "Swarm.h"
#include "Engine.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "BaseGun.generated.h"

USTRUCT(BlueprintType)
struct FWeaponStats
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades") float Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades") float Accuracy;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades") float FireRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades") float MagazineSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades") float Lifesteal;
};

class ASwarmCharacter;

UCLASS()
class SWARM_API ABaseGun : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseGun(const class FObjectInitializer& PCIP);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	bool GetIsShooting();
	bool GetIsReloading();

	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	int32 GetCurrentUpgradePercent(int32 Stat);

	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	float GetUpgradeStepValue(int32 Stat);

	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	void AddUpgrade(int32 Stat);

	UFUNCTION(BlueprintImplementableEvent, Category = "Sounds")
	void PlayImpactSound(FVector ImpactPosition);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = StaticMeshComponents, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "State")
	bool IsShooting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "State")
	bool IsReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "State")
	int32 CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Stats")
	int32 TotalAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Stats")
	int32 MaxClipAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ReloadTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ProjectileSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ShootTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseShootTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ProjectileGravityScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool UseProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 Weight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FHitResult ShootHitResult;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Stats")
	UClass* ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Stats")
	class ABaseProjectile* Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool IsAutomatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool IsMelee;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 InventorySlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Stats")
	EWeaponType GunType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Effects)
	TArray<UParticleSystem*> ParticleTemplates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	FWeaponStats WeaponStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	TArray<FWeaponStats> WeaponStatsMinMax;

	// Functions
	void FireBullet();

	FHitResult ShootHandler();

	UFUNCTION(Category = "Weapon System")
	void ReloadCall();

	UFUNCTION(Category = "Weapon System")
	void StartReload();

	UFUNCTION(Category = "Weapon System")
	void FinishReload();

	UFUNCTION(NetMulticast, Reliable, Category = "Weapon System")
	void MulticastImpact(FVector _Impact, int32 ImpactID);

	void MulticastImpact_Implementation(FVector _Impact, int32 ImpactID);

};
