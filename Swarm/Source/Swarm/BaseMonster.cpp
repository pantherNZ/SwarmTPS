// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseMonster.h"
#include "Swarm.h"
#include "BaseGun.h"
#include "SwarmCharacter.h"
#include "BaseGameState.h"
#include "SwarmGameMode.h"
#include "BaseProjectile.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABaseMonster::ABaseMonster(const class FObjectInitializer& PCIP)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialise variables
	Health = 100.0f;
	AttackDistance = 175.0f;
	Damage = 10.0f;
	State = EMonsterState::EMS_CHASING;
	MonsterType = EMonsterType::EMT_SWARMER_SMALL;
	HitParticle = nullptr;
	KillReward = 100;
	Leadership = 0.0f;

	Collision = PCIP.CreateDefaultSubobject<UBoxComponent>(this, "Box Collision");
	Collision->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Collision->SetGenerateOverlapEvents(true);

	GibBlueprint = nullptr;
	DeathPosition = FVector(0.0f, 0.0f, 0.0f);
	DeathStrength = 0.0f;
}

// Called when the game starts or when spawned
void ABaseMonster::BeginPlay()
{
	Super::BeginPlay();

	Initalise();
}

// Called every frame
void ABaseMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseMonster::RotateToMovement()
{
	FHitResult HitResult = this->GetCharacterMovement()->CurrentFloor.HitResult;

	if (HitResult.bBlockingHit)
	{
		if (this->GetVelocity().Size() > 0.001f)
		{
			FRotator Rotation = UKismetMathLibrary::MakeRotFromZX(HitResult.Normal, this->GetVelocity());
			float fAlpha = this->GetCharacterMovement()->RotationRate.Yaw;
			Rotation = UKismetMathLibrary::RInterpTo(GetActorRotation(), Rotation, GetWorld()->GetDeltaSeconds(), fAlpha / 20.0f);
			SetActorRotation(Rotation);
		}
	}
}

void ABaseMonster::RotateToTarget()
{
	FHitResult HitResult = this->GetCharacterMovement()->CurrentFloor.HitResult;

	if (HitResult.bBlockingHit)
	{
		FVector Direction = Target->GetActorLocation() - this->GetActorLocation();
		Direction.Normalize();
		FRotator Rotation = UKismetMathLibrary::MakeRotFromZX(HitResult.Normal, Direction);
		SetActorRotation(Rotation);
	}
}

// Called to bind functionality to input
void ABaseMonster::SetupPlayerInputComponent(UInputComponent* inputComponent)
{
	Super::SetupPlayerInputComponent(inputComponent);

}

float ABaseMonster::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser)
{
	if (Health > 0.0f)
	{
		ABaseGun* TempGun = Cast<ABaseGun>(DamageCauser);
		ASwarmCharacter* pCauser = NULL;
		bool bProjectile = false;

		if (IsValid(TempGun))
		{
			pCauser = Cast<ASwarmCharacter>(TempGun->GetOwner());

			if (IsValid(pCauser))
			{
				pCauser->ConfirmHit(DamageAmount);
				DeathPosition = pCauser->GetActorLocation();
				float fDist = FVector::Dist(this->GetActorLocation(), DeathPosition);
				DeathStrength = UKismetMathLibrary::FClamp(3000.0f - fDist, 0.0f, 3000.0f) / 1500.0f;
			}
		}
		else
		{
			ABaseProjectile* pProj = Cast<ABaseProjectile>(DamageCauser);

			if (IsValid(pProj))
			{
				bProjectile = true;
				DeathPosition = pProj->GetActorLocation();
				float fDist = FVector::Dist(this->GetActorLocation(), DeathPosition);
				DeathStrength = UKismetMathLibrary::FClamp(800.0f - fDist, 0.0f, 800.0f) / 300.0f;
				pCauser = Cast<ASwarmCharacter>(pProj->GetOwner());

				if (IsValid(pCauser))
				{
					pCauser->ConfirmHit(DamageAmount, pProj->GetUniqueID());
				}
			}
		}

		#if !UE_BUILD_SHIPPING
		if (!IsValid(pCauser))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Monster: TakeDamage - Causer not a player");
		}
		#endif

		Health -= DamageAmount;

		FVector DirectionVector = (GetActorLocation() - DamageCauser->GetActorLocation());
		DirectionVector.Normalize();

		SpawnDecal(DirectionVector);

		if (Health <= 0.0f)
		{
			if (GetMesh()->bRenderCustomDepth)
			{
				ASwarmGameMode* pGameMode = Cast<ASwarmGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

				if (IsValid(pGameMode))
				{
					pGameMode->MonstersHighlighted--;
					pGameMode->CheckHighlight();
				}
			}

			if (IsValid(pCauser))
			{
				pCauser->ConfirmKill(KillReward, this->GetActorLocation());
			}
			else
			{
				GiveMoneyToPlayers(KillReward, this->GetActorLocation());
			}

			ABaseGameState* pGameState = GetWorld()->GetGameState<ABaseGameState>();
			int32 iCount = 0;

			while (true)
			{
				++iCount;

				if (IsValid(pGameState))
				{
					pGameState->CurrentWaveKills++;
					pGameState->MonstersAlive--;
					#if !UE_BUILD_SHIPPING
						GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::FromInt(pGameState->MonstersAlive));
					#endif
					break;
				}
				else if (iCount >= 10)
				{	
					#if !UE_BUILD_SHIPPING
						GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, "Major Issue - Game state not valid in BaseMonster.cpp");
					#endif
					break;
				}	
			} 

			// Spawn gibs
			if (IsValid(GibBlueprint))
			{
				FVector Location = this->GetActorLocation();
				FRotator Rotation = this->GetActorRotation();
				FActorSpawnParameters SP;
				SP.Owner = this;
				SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				GetWorld()->SpawnActor(GibBlueprint, &Location, &Rotation, SP);
				//this->Destroy();
				this->SetLifeSpan(0.01f);
			}
			else
			{
				this->SetLifeSpan(5.0f);
			}

			Death();

			if (IsValid(this->GetController()))
			{
				this->GetController()->StopMovement();
			}

			this->GetCharacterMovement()->StopActiveMovement();
			this->State = EMonsterState::EMS_DEAD;
		}
	}

	return(DamageAmount);
}

void ABaseMonster::Death_Implementation()
{
	FVector vec3Position = GetActorLocation();

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, vec3Position);

	GetCharacterMovement()->bUseRVOAvoidance = false;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	//GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	ABaseGameState* pGameState = Cast<ABaseGameState>(GetWorld()->GetGameState());

	if (IsValid(pGameState) && (pGameState->MonstersMax - pGameState->CurrentWaveKills <= 7))
	{
		for (TActorIterator<ABaseMonster> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			(*ActorItr)->ShowMonsterOutline();

			CheckOutlineTimer();
		}
	}
}

bool ABaseMonster::Death_Validate()
{
	return(true);
}

AActor* ABaseMonster::GetCurrentTarget(bool& Success)
{
	if (IsValid(Target))
	{
		Success = true;
		return(Target);
	}

	Success = false;
	return(nullptr);
}

void ABaseMonster::DealDamage(bool LaunchPlayer)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
		FDamageEvent DamageEvent(ValidDamageTypeClass);

		bool bSuccess;
		AActor* pTarg = GetCurrentTarget(bSuccess);

		if (bSuccess)
		{
			pTarg->TakeDamage(Damage, DamageEvent, GetController(), this);

			ASwarmCharacter* pTargPly = Cast<ASwarmCharacter>(pTarg);

			if (LaunchPlayer && IsValid(pTargPly))
			{
				FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(this->GetActorLocation(), pTargPly->GetActorLocation());
				Direction *= 1500.0f;
				Direction += FVector(0.0f, 0.0f, 1500.0f);
				pTargPly->LaunchCharacter(Direction, false, false);
			}
		}
	}
}

void ABaseMonster::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseMonster, Health);
	DOREPLIFETIME(ABaseMonster, State);
	DOREPLIFETIME(ABaseMonster, HitParticle);
}