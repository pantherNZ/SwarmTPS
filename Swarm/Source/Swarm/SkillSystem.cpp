// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillSystem.h"
#include "Swarm.h"
#include "Net/UnrealNetwork.h"
#include "BasePlayerState.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASkillSystem::ASkillSystem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);
	bAlwaysRelevant = true;

	CurrentSkill = ESkillType::EST_NONE;

	FPassiveSkills MinStats;
	FPassiveSkills MaxStats;
	PassiveSkillsMinMax.Add(MinStats);
	PassiveSkillsMinMax.Add(MaxStats);

	TeleportPlaceParticle = nullptr;
	TeleportParticle = nullptr;
}

// Called when the game starts or when spawned
void ASkillSystem::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimerForNextTick(this, &ASkillSystem::Initialise);
}

void ASkillSystem::Initialise()
{
	PassiveSkills.MovementSpeed = Cast<ASwarmCharacter>(this->GetOwner())->RunSpeed;
	PassiveSkills.HealthRegen = 0.0f;
	PassiveSkills.ReviveSpeed = Cast<ASwarmCharacter>(this->GetOwner())->ReviveSpeed;
	PassiveSkills.JumpHeight = Cast<ASwarmCharacter>(this->GetOwner())->GetCharacterMovement()->JumpZVelocity;

	PassiveSkillsMinMax[0] = PassiveSkills;

	NumMines = 5;
}

// Called every frame
void ASkillSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ASkillSystem::SkillSlotFull()
{
	return(CurrentSkill != ESkillType::EST_NONE);
}

void ASkillSystem::ExecuteSkill()
{
	
	switch (CurrentSkill)
	{
	case ESkillType::EST_TELEPORT_PLACE:
	{
		ASwarmCharacter* pOwner = Cast<ASwarmCharacter>(this->GetOwner());

		if (pOwner->IsAlive())
		{
			TeleportPosition = pOwner->GetActorLocation();
			CurrentSkill = ESkillType::EST_TELEPORT;
			FHitResult HitResult = pOwner->GetCharacterMovement()->CurrentFloor.HitResult;
			FVector Pos = TeleportPosition - FVector(0.0f, 0.0f, 120.0f);/// (HitResult.bBlockingHit ? HitResult.Location : TeleportPosition - FVector(0.0f, 0.0f, 80.0f));
			FRotator Rotation = UKismetMathLibrary::MakeRotFromZ(HitResult.Normal);
			pOwner->SpawnParticleMulticast(this->TeleportPlaceParticle, FVector(0.2f, 0.2f, 0.2f), Pos, 0, Rotation);
		}
		break;
	}

	case ESkillType::EST_TELEPORT:
	{
		ASwarmCharacter* pOwner = Cast<ASwarmCharacter>(this->GetOwner());

		if (pOwner->IsAlive())
		{
			FVector Pos = pOwner->GetActorLocation() - FVector(0.0f, 0.0f, 120.0f);
			pOwner->SpawnParticleMulticast(this->TeleportParticle, FVector(0.8f, 0.8f, 0.8f), Pos, -1);
			pOwner->TeleportTo(TeleportPosition, this->GetOwner()->GetActorRotation());

			Pos = TeleportPosition - FVector(0.0f, 0.0f, 120.0f);
			pOwner->SpawnParticleMulticast(this->TeleportParticle, FVector(0.8f, 0.8f, 0.8f), Pos, -1);
			pOwner->DeactivateParticleMulticast(0);

			CurrentSkill = ESkillType::EST_NONE;
		}
		break;
	}

	case ESkillType::EST_SELFREVIVE:
	{
		ASwarmCharacter* pChar = Cast<ASwarmCharacter>(this->GetOwner());

		if (!pChar->IsAlive())
		{
			pChar->RevivePlayer();
			CurrentSkill = ESkillType::EST_NONE;
		}

		break;
	}

	case ESkillType::EST_MINEPLACEMENT:
	{
		ASwarmCharacter* pChar = Cast<ASwarmCharacter>(this->GetOwner());
		FHitResult hitResult;

		if (IsValid(pChar) && pChar->IsAlive())
		{
			//FVector MinePlacementPosition = pChar->GetActorLocation();
			FRotator MineRotation(0.0f, 0.0f, 0.0f);

			if (GetLocalRole() == ROLE_Authority)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "debug msgOne");
				if (NumMines > 0)
				{
					if (pChar->CharacterInteractCheck(ECC_WorldStatic, hitResult, 1000.0f))
					{
						//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "debug msgTwo");
						AActor* Mine = GetWorld()->SpawnActor(Mines, &hitResult.Location, &MineRotation);
						Mine->SetOwner(pChar);

						NumMines -= 1;
					}
				}
				else
				{
					CurrentSkill = ESkillType::EST_NONE;
					NumMines = 5;
				}
			}
		}

		break;
	}

	case ESkillType::EST_MEDKIT:
	{
		ASwarmCharacter* pChar = Cast<ASwarmCharacter>(this->GetOwner());

		if (IsValid(pChar) && pChar->IsAlive())
		{
			ABasePlayerState* PS = Cast<ABasePlayerState>(pChar->GetPlayerState());
			float fHealthGain = (100.0f - PS->Health) * 0.75f;
			PS->UpdateHealth(fHealthGain);
			pChar->SpawnParticleMulticast(this->MedkitParticle, FVector(2.0f, 2.0f, 2.0f), pChar->GetActorLocation(), -1);
			CurrentSkill = ESkillType::EST_NONE;
		}

		break;
	}

	}
}

int32 ASkillSystem::GetCurrentUpgradePercent(int32 Stat)
{
	switch (Stat)
	{
	case(0) :
	{
		float fIncreased = (PassiveSkills.MovementSpeed - PassiveSkillsMinMax[0].MovementSpeed);
		float fGap = (PassiveSkillsMinMax[1].MovementSpeed - PassiveSkillsMinMax[0].MovementSpeed);
		return(FMath::RoundToInt((fIncreased / fGap) * 100.0f));
	}
	case(1) :
	{
		float fIncreased = (PassiveSkills.HealthRegen - PassiveSkillsMinMax[0].HealthRegen);
		float fGap = (PassiveSkillsMinMax[1].HealthRegen - PassiveSkillsMinMax[0].HealthRegen);
		return(FMath::RoundToInt((fIncreased / fGap) * 100.0f));
	}
	case(2) :
	{
		float fIncreased = (PassiveSkills.ReviveSpeed - PassiveSkillsMinMax[0].ReviveSpeed);
		float fGap = (PassiveSkillsMinMax[1].ReviveSpeed - PassiveSkillsMinMax[0].ReviveSpeed);
		return(FMath::RoundToInt((fIncreased / fGap) * 100.0f));
	}
	case(3) :
	{
		float fIncreased = (PassiveSkills.JumpHeight - PassiveSkillsMinMax[0].JumpHeight);
		float fGap = (PassiveSkillsMinMax[1].JumpHeight - PassiveSkillsMinMax[0].JumpHeight);
		return(FMath::RoundToInt((fIncreased / fGap) * 100.0f));
	}
	};

	return(0);
}

float ASkillSystem::GetUpgradeStepValue(int32 Stat)
{
	switch (Stat)
	{
	case(0) :
		return((PassiveSkillsMinMax[1].MovementSpeed - PassiveSkillsMinMax[0].MovementSpeed) / 10.0f);
	case(1) :
		return((PassiveSkillsMinMax[1].HealthRegen - PassiveSkillsMinMax[0].HealthRegen) / 10.0f);
	case(2) :
		return((PassiveSkillsMinMax[1].ReviveSpeed - PassiveSkillsMinMax[0].ReviveSpeed) / 10.0f);
	case(3) :
		return((PassiveSkillsMinMax[1].JumpHeight - PassiveSkillsMinMax[0].JumpHeight) / 10.0f);
	};

	return(0.0f);
}

void ASkillSystem::SetSkill(ESkillType SkillType, FString Name)
{
	if (SkillType == ESkillType::EST_TELEPORT) SkillType = ESkillType::EST_TELEPORT_PLACE;

	CurrentSkill = SkillType;
	CurrentSkillName = Name;
}

void ASkillSystem::UpgradePassive(ESkillType SkillType)
{
	switch (SkillType)
	{
	case ESkillType::EST_MOVEMENTSPEED:
	{
		PassiveSkills.MovementSpeed += GetUpgradeStepValue(0);
		ASwarmCharacter* pChar = Cast<ASwarmCharacter>(this->GetOwner());
		pChar->SprintSpeed = PassiveSkills.MovementSpeed + (pChar->SprintSpeed - pChar->RunSpeed);
		pChar->BatteryRunSpeed = PassiveSkills.MovementSpeed + (pChar->BatteryRunSpeed - pChar->RunSpeed);
		pChar->RunSpeed = PassiveSkills.MovementSpeed;
		
		break;
	}
	case ESkillType::EST_HEALTHREGEN:
	{
		PassiveSkills.HealthRegen += GetUpgradeStepValue(1);
		break;
	}
	case ESkillType::EST_REVIVESPEED:
	{
		PassiveSkills.ReviveSpeed += GetUpgradeStepValue(2);
		Cast<ASwarmCharacter>(this->GetOwner())->ReviveSpeed = PassiveSkills.ReviveSpeed;
		break;
	}
	case ESkillType::EST_JUMPHEIGHT:
	{
		PassiveSkills.JumpHeight += GetUpgradeStepValue(3);
		Cast<ASwarmCharacter>(this->GetOwner())->GetCharacterMovement()->JumpZVelocity = PassiveSkills.JumpHeight;
		break;
	}
	};
}

void ASkillSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASkillSystem, CurrentSkill);
	DOREPLIFETIME(ASkillSystem, PassiveSkills);
	DOREPLIFETIME(ASkillSystem, PassiveSkillsMinMax);
}