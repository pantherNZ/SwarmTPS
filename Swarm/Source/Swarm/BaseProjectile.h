// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseGun.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BaseProjectile.generated.h"

UCLASS()
class SWARM_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseProjectile();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Collision")
	bool CheckIfHit(AActor* OtherActor);

	UFUNCTION(NetMulticast, Reliable, Category = "Collision")
	void MulticastSpawnParticle();

	void MulticastSpawnParticle_Implementation();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Base, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Projectile;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Base, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* Movement;

	UFUNCTION(BlueprintImplementableEvent, Category = "Collision")
	void ApplyVelocity(FVector VelocityInLocalSpace, bool Activate, float GravityScale, float ProjectileSpeed);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UParticleSystem* ImpactParticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool Activated;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		ABaseGun* Gun;
};
