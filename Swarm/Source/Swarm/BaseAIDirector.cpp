// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseAIDirector.h"
#include "Swarm.h"
#include "BaseGameState.h"
#include "BaseMonster.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"

// Sets default values
ABaseAIDirector::ABaseAIDirector()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MinWaveSpitter = 2;
	MinWaveSlammer = 4;
	MinWaveBroodmother = 6;
	MonstersSpawned = 0;

	SwarmerSpawnRatio = 100.0f;
	SpitterSpawnRatio = 0.0f;
	SlammerSpawnRatio = 0.0f;
	BroodmotherSpawnRatio = 0.0f;

	MonsterLimit = 80;
}

// Called when the game starts or when spawned
void ABaseAIDirector::BeginPlay()
{
	Super::BeginPlay();

	m_fTimer = 0.0f;
	m_fInterval = FMath::FRandRange(0.1f, 1.0f);
	GetWorldTimerManager().SetTimerForNextTick(this, &ABaseAIDirector::GetSpawnPoints);
}

void ABaseAIDirector::GetSpawnPoints()
{
	for (TActorIterator<ABaseSpawnPoint> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if ((*ActorItr)->BroodmotherSP) m_SpawnPointsBroodmother.Add(*ActorItr);
		else m_SpawnPointsRegular.Add(*ActorItr);	
	}
}

// Called every frame
void ABaseAIDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() == ROLE_Authority)
	{
		ABaseGameState* pGameState = Cast<ABaseGameState>(GetWorld()->GetGameState());

		if (IsValid(pGameState))
		{
			//if (Monsters.Num() > 0)
			//{
			if (pGameState->GameState == EGameState::EGS_WAVEACTIVE &&
				MonstersSpawned < pGameState->MonstersMax &&
				pGameState->MonstersAlive < MonsterLimit)
				//MonstersSpawned - pGameState->CurrentWaveKills < MonsterLimit)
			{
				m_fTimer += DeltaTime;

				if (m_fTimer >= m_fInterval)
				{
					m_fInterval = FMath::FRandRange(0.1f, 0.2f);
					SpawnMonster(pGameState->WaveNumber);
					m_fTimer = 0.0f;
				}
			}
		}
	}
}

int32 ABaseAIDirector::WaveEnd(int32 WaveNumber, int32 MonsterPoints)
{
	Monsters.Empty();

	int32 MonstersCount = 0;
	MonstersSpawned = 0;

	SwarmerSpawnRatio = FMath::Clamp(SwarmerSpawnRatio - 2.0f, 80.0f, 100.0f);

	if (WaveNumber >= MinWaveSpitter)
	{
		SpitterSpawnRatio = FMath::Clamp(SpitterSpawnRatio + 1.0f, 0.0f, 8.0f);
	}

	if (WaveNumber >= MinWaveSlammer)
	{
		SlammerSpawnRatio = FMath::Clamp(SlammerSpawnRatio + 0.5f, 0.0f, 2.0f);
	}

	if (WaveNumber >= MinWaveBroodmother)
	{
		float fBroodChance = UKismetMathLibrary::RandomFloatInRange(0.1f, float(WaveNumber) * 0.2f);
		BroodmotherSpawnRatio = (UKismetMathLibrary::RandomIntegerInRange(0, 4) == 0 ? 0.0f : fBroodChance);
	}

	EMonsterType eType = EMonsterType::EMT_BROODMOTHER;

	float SwarmerSmall = 0.2f * SwarmerSpawnRatio;
	float SwarmerMedium = 0.4f * SwarmerSpawnRatio;
	float SwarmerLarge = 0.4f * SwarmerSpawnRatio;

	float Total = SwarmerSpawnRatio + SpitterSpawnRatio + SlammerSpawnRatio + BroodmotherSpawnRatio;

	// Determine monster counts
	while (MonsterPoints >= 0)
	{
		float Random = FMath::FRandRange(0.0f, Total);

		if (Random < SwarmerSmall)
		{
			eType = EMonsterType::EMT_SWARMER_SMALL;
			MonsterPoints -= 1;
		}
		else if (Random < SwarmerMedium + SwarmerSmall)
		{
			eType = EMonsterType::EMT_SWARMER_MEDIUM;
			MonsterPoints -= 1;
		}
		else if (Random < SwarmerLarge + SwarmerMedium + SwarmerSmall)
		{
			eType = EMonsterType::EMT_SWARMER_LARGE;
			MonsterPoints -= 1;
		}
		else if (Random < SpitterSpawnRatio + SwarmerLarge + SwarmerMedium + SwarmerSmall)
		{
			eType = EMonsterType::EMT_SPITTER;
			MonsterPoints -= 4;
		}
		else if (Random < SpitterSpawnRatio + SwarmerLarge + SwarmerMedium + SwarmerSmall + SlammerSpawnRatio)
		{
			eType = EMonsterType::EMT_SLAMMER;
			MonsterPoints -= 10;
		}
		else
		{
			Total -= BroodmotherSpawnRatio;
			MonsterPoints -= 15;
			eType = EMonsterType::EMT_BROODMOTHER;
		}

		Monsters.Add(eType);
		MonstersCount++;
	}

	return(MonstersCount);
}

void ABaseAIDirector::SpawnMonster(int32 WaveNumber)
{
	if (Monsters.Num() > 0)
	{
		EMonsterType eType = Monsters[Monsters.Num() - 1];

		#if !UE_BUILD_SHIPPING
		if (eType == EMonsterType::EMT_BROODMOTHER)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, "Spawning Broodmother");
		}
		#endif

		// Find random spawn point
		ABaseSpawnPoint* SpawnPt = nullptr;
		int iIterations = 0;

		do
		{
			TArray<ABaseSpawnPoint*>* pArray = &(eType == EMonsterType::EMT_BROODMOTHER ? m_SpawnPointsBroodmother : m_SpawnPointsRegular);

			int32 iRandomSpawn = FMath::RandRange(0, (*pArray).Num() - 1);
			SpawnPt = (*pArray)[iRandomSpawn];

			iIterations++;

			if (iIterations >= 20)
			{
#if !UE_BUILD_SHIPPING
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Spawn attempt timeout");
#endif
				return;
			}

			if (eType == EMonsterType::EMT_BROODMOTHER && SpawnPt->BroodmotherSP)
			{
				break;
			}
		} 
		while ((eType == EMonsterType::EMT_BROODMOTHER && !SpawnPt->BroodmotherSP) || !IsValid(SpawnPt) || !SpawnPt->CanSpawn());

		FNavLocation NavigationInfo;
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

		if (!NavSys || !NavSys->GetRandomPointInNavigableRadius(SpawnPt->GetActorLocation(), 500.0f, NavigationInfo))
		{
#if !UE_BUILD_SHIPPING
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Failed to find valid spawn point");
#endif
			return;
		}

		// Spawn Pos
		FVector SpawnPosition = NavigationInfo.Location;

		FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);
		float fScale = 2.0f * ((FMath::Sin(2 * PI * (FMath::FRand() - 0.25f)) + 1) / 3.0f + 0.7f);
		FVector SpawnScale = FVector(fScale, fScale, fScale);

		// Call the overridden blueprint function to spawn the monster (handles spawning the particular type of monster, which this base class cannot access)
		SpawnMonsterInWorld(SpawnPosition, SpawnRotation, SpawnScale, eType);
	}
}