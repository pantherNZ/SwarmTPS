// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseProjectile.h"
#include "BaseGameState.h"
#include "Swarm.h"


// Sets default values
ABaseProjectile::ABaseProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	Projectile = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile"));
	Projectile->AttachTo(RootComponent);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));

	SetActorEnableCollision(true);
}

// Called when the game starts or when spawned
void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ABaseProjectile::CheckIfHit(AActor* OtherActor)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (Activated && IsValid(Gun))
		{
			if (Gun->GetOwner() != OtherActor)
			{		
				MulticastSpawnParticle();
			//	Destroy();

				TArray<AActor*> IgnoreActors;
				UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), Gun->WeaponStats.Damage, 1.0f, GetActorLocation(), 50.0f, 600.0f, 1.0f, NULL, IgnoreActors, this);

				return true;
			}
		}
	}

	return false;
}

void ABaseProjectile::MulticastSpawnParticle_Implementation()
{
	UParticleSystemComponent* Impact = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, Projectile->GetComponentLocation());
	Impact->SetRelativeScale3D(FVector(3.0, 3.0, 3.0));
}