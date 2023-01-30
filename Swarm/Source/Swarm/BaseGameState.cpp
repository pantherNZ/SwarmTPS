// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseGameState.h"
#include "Swarm.h"
#include "BasePlayerState.h"
#include "BaseAIDirector.h"
#include "SwarmGameMode.h"
#include "SwarmCharacter.h"
#include "BaseMonster.h"
#include "Utility.h"
#include "NavigationSystem.h"

ABaseGameState::ABaseGameState()
{
	WaveNumber = 1;
	CurrentWaveKills = 0;
	TotalKills = 0;
	WaveTimer = 10.0f;
	WaveIntervalLength = 60.0f;
	GameState = EGameState::EGS_MENU;
	MonstersMax = 40;
	MonsterPoints = 40;
	HealthLostDuringWave = 0;
	Decreased = 0;
	MonstersAlive = 0;
	InStartLobby = true;
	Difficulty = 1.0f;
}

void ABaseGameState::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimerForNextTick(this, &ABaseGameState::GetAIDirector);
}

void ABaseGameState::GetAIDirector()
{
	for (TActorIterator<ABaseAIDirector> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AIDirector = (*ActorItr);
		break;
	}
}

void ABaseGameState::UpdateActualMonsterCount()
{
	//if (MonstersMax - CurrentWaveKills <= 5)
	//{
		MonstersAlive = 0;

		for (TActorIterator<ABaseMonster> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			MonstersAlive++;
		}

		#if !UE_BUILD_SHIPPING
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, FString::FromInt(MonstersAlive));
		#endif

		GetWorldTimerManager().SetTimer(CheckMonsterCountTimer, this, &ABaseGameState::UpdateActualMonsterCount, 1.0f, false);
	//}
}

void ABaseGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() == ROLE_Authority)
	{
		if (GameState == EGameState::EGS_WAVEINTERVAL)
		{
			CheckGameOver();

			WaveTimer -= DeltaTime;

			if (WaveTimer <= 0.0f || CheckPlayersReady())
			{
				WaveTimer = 0.0f;
				GameState = EGameState::EGS_WAVEACTIVE;

				ASwarmGameMode* GameMode = Cast<ASwarmGameMode>(GetWorld()->GetAuthGameMode());
				GameMode->CanHighlight = true;
				GameMode->MonstersHighlighted = 0;

				// Tell players to close their ready up widgets
				for (TActorIterator<ASwarmCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
				{
					(*ActorItr)->GameStarting();
				}

				for (auto& iter : PlayerArray)
				{
					ABasePlayerState* PS = dynamic_cast<ABasePlayerState*>(iter);

					if (IsValid(PS))
					{
						PS->Ready = false;
					}
				}
			}
		}
		else if (GameState == EGameState::EGS_WAVEACTIVE)
		{
			CheckGameOver();

			WaveTime += DeltaTime;
			InStartLobby = false;

			// Wave ended (killed all enemies)
			if (MonstersAlive <= 0 && AIDirector->MonstersSpawned >= MonstersMax)
			{
				GetWorldTimerManager().ClearTimer(CheckMonsterCountTimer);

				TotalKills += CurrentWaveKills;
				CurrentWaveKills = 0;

				int iPlayers = PlayerArray.Num();

				// Monster count increase
				MonsterPoints += ((WaveNumber * 8 + iPlayers * 4 + FMath::RandRange(-WaveNumber * 5, WaveNumber * 5)) / 2) * Difficulty;

				WaveTime = 0.0f;
				WaveTimer = WaveIntervalLength;
				GameState = EGameState::EGS_WAVEINTERVAL;
				HealthLostDuringWave = 0;
				WaveNumber++;

				if (!IsValid(AIDirector)) GetAIDirector();

				MonstersMax = AIDirector->WaveEnd(WaveNumber, MonsterPoints);
				MonstersAlive = 0;

				for (auto& iter : PlayerArray)
				{
					ABasePlayerState* PS = dynamic_cast<ABasePlayerState*>(iter);

					if (IsValid(PS))
					{
						PS->WaveEndClient();
					}
				}

				if (WaveNumber % 2 == 0)
				{
					for (TActorIterator<ASwarmCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
					{
						if (!(*ActorItr)->IsAlive() && (*ActorItr)->Lives == 0)
						{
							ABasePlayerState* PS = Cast<ABasePlayerState>((*ActorItr)->GetPlayerState());
							PS->Health = 100.0f;
							(*ActorItr)->Lives = 5;
							(*ActorItr)->SetActorHiddenInGame(false);
							(*ActorItr)->SwitchToRegularCameraClient();
							//(*ActorItr)->GetMesh()->bRenderCustomDepth = false;
							//ASwarmCharacter* pNewChar = Cast<ASwarmCharacter>(GetWorld()->SpawnActor(CharacterClass));

							FNavLocation NavigationInfo;
							UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

							if (NavSys->GetRandomReachablePointInRadius(FVector(650.0f, 50.0f, -130.0f), 200.0f, NavigationInfo))
							{
								(*ActorItr)->TeleportTo(NavigationInfo.Location, FRotator(0.0f, 0.0f, 0.0f));
							}
						}
					}
				}
			}
			else if (!GetWorldTimerManager().TimerExists(CheckMonsterCountTimer)) // MonstersMax - CurrentWaveKills <= 5 && 
			{
				//GetWorldTimerManager().SetTimer(CheckMonsterCountTimer, this, &ABaseGameState::UpdateActualMonsterCount, 1.0f, false);
			}
		}
		else if (GameState != EGameState::EGS_GAMEOVER)
		{
			if (PlayerArray.Num() >= 1)
			{
				GameState = EGameState::EGS_PLAYERSNOTREADY;

				if (CheckPlayersReady())
				{
					if (!IsValid(AIDirector)) GetAIDirector();

					MonstersMax = AIDirector->WaveEnd(WaveNumber, 10 * PlayerArray.Num());
					GameState = EGameState::EGS_WAVEINTERVAL;

					GameStart();
				}
			}
			else
			{
				GameState = EGameState::EGS_WAITINGFORPLAYERS;
			}
		}
	}
}

bool ABaseGameState::CheckPlayersReady()
{
	PlayerReadyCount = 0;

	for (auto& iter : PlayerArray)
	{
		ABasePlayerState* PS = dynamic_cast<ABasePlayerState*>(iter);

		if (IsValid(PS))
		{
			PlayerReadyCount += (PS->Ready ? 1 : 0);
		}
	}

	if (PlayerReadyCount >= PlayerArray.Num())
	{
		// Tell players to close their ready up widgets
		for (TActorIterator<ASwarmCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			(*ActorItr)->GameStarting();
		}

		for (auto& iter : PlayerArray)
		{
			ABasePlayerState* PS = dynamic_cast<ABasePlayerState*>(iter);

			if (IsValid(PS))
			{
				PS->Ready = false;
			}
		}

		return(true);
	}

	return(false);
}

void ABaseGameState::CheckGameOver()
{
	// Players dead
	bool Alive = false;

	for (auto& iter : PlayerArray)
	{
		ABasePlayerState* PS = dynamic_cast<ABasePlayerState*>(iter);

		if (IsValid(PS) && PS->Health > 0.0f)
		{
			Alive = true;
			break;
		}
	}

	if (!Alive)
	{
		GameState = EGameState::EGS_GAMEOVER;
	}
}

bool ABaseGameState::InLobby()
{
	return(GameState == EGameState::EGS_PLAYERSNOTREADY ||
		GameState == EGameState::EGS_WAITINGFORPLAYERS ||
		InStartLobby);
}

void ABaseGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseGameState, WaveNumber);
	DOREPLIFETIME(ABaseGameState, PlayerReadyCount);
	DOREPLIFETIME(ABaseGameState, CurrentWaveKills);
	DOREPLIFETIME(ABaseGameState, MonstersMax);
	DOREPLIFETIME(ABaseGameState, TotalKills);
	DOREPLIFETIME(ABaseGameState, WaveTimer);
	DOREPLIFETIME(ABaseGameState, GameState);
	DOREPLIFETIME(ABaseGameState, InStartLobby);
	DOREPLIFETIME(ABaseGameState, MonstersAlive);
}