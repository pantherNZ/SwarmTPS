// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseSpawnPoint.h"
#include "Swarm.h"
#include "SwarmCharacter.h"

// Sets default values
ABaseSpawnPoint::ABaseSpawnPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	IsEnabled = true;
	BroodmotherSP = false;
}

// Called when the game starts or when spawned
void ABaseSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABaseSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ABaseSpawnPoint::CanSpawn()
{
	return(IsEnabled && !HasPlayerLOS());
}

bool ABaseSpawnPoint::HasPlayerLOS()
{
	for (TActorIterator<ASwarmCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		// Line trace for LOS
		bool bBehindObject = false;
		FHitResult hitResult;
		FCollisionQueryParams params = FCollisionQueryParams(FName(TEXT("CanSpawnTrace")), false, this);

		GetWorld()->LineTraceSingleByChannel(hitResult, this->GetActorLocation(), (*ActorItr)->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f), ECC_Visibility, params);

		//DrawDebugLine(GetWorld(), this->GetActorLocation(), hitResult.Location, FColor(255, 0, 0), true, -1, 0, 3);

		if (hitResult.bBlockingHit && IsValid(hitResult.Actor.Get()))
		{
			bBehindObject = hitResult.Actor.Get() != (*ActorItr);
		}

		// Angle for LOS
		FVector vecDir1 = this->GetActorLocation() - ((*ActorItr)->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f));
		vecDir1.Z = 0.0f;
		vecDir1.Normalize();
		FVector vecDir2 = (*ActorItr)->GetActorForwardVector();
		vecDir2.Z = 0.0f;
		float fAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(vecDir1, vecDir2)));
		bool bInLineOfSight = fAngle <= 60.0f;

		// Distance
		float fDistance = this->GetDistanceTo(*ActorItr);

		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, (*ActorItr)->PlayerState->PlayerName + " " + FString::SanitizeFloat(fDistance));
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, bBehindObject ? "TRUE" : "FALSE");
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, bInLineOfSight ? "TRUE" : "FALSE");

		// Checks
		if (fDistance <= 1000.0f) return(true);
		if (!bBehindObject && bInLineOfSight) return(true);
	}

	return(false);
}