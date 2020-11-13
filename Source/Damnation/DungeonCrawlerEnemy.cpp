// Fill out your copyright notice in the Description page of Project Settings.

#include "DungeonCrawlerEnemy.h"
#include "DungeonCrawlerPlayer.h"
#include "DamnationGameModeBase.h"

// Sets default values
ADungeonCrawlerEnemy::ADungeonCrawlerEnemy()
{
	Type = EOccupantType::Enemy;
	RepathInterval = FMath::RandRange(0, MaxRepathInterval);

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Gamemode = dynamic_cast<ADamnationGameModeBase*>(UGameplayStatics::GetGameMode(GetWorld()));
	if (Gamemode)
		Gamemode->ActiveEnemies.Add(this);
}

// Called when the game starts or when spawned
void ADungeonCrawlerEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADungeonCrawlerEnemy::PerformMovement()
{
	// Update oldpos to match current pos so lagged root doesn't wig out
	// Will be properly set later if necessary
	OldTransform = GetActorTransform();

	ReceiveMoveAction();
}

void ADungeonCrawlerEnemy::SetTarget(ADungeonSingleTile* Target)
{
	DesiredPath = Gamemode->CallPathfinder(CurrentTile, Target, 1, true, true);
}

void ADungeonCrawlerEnemy::SetModelFacing(ECardinal direction)
{
	FRotator newRotation = FRotator::ZeroRotator;
	newRotation.Add(0, (float)((int)direction * 90), 0);
	SetActorRotation(newRotation, ETeleportType::TeleportPhysics);
}

void ADungeonCrawlerEnemy::AlertTarget(ADungeonSingleTile* Target)
{
	// Post-Decrement repath interval; if <= 0, repath
	if (RepathInterval-- <= 0 || DesiredPath.Num() == 0)
	{
		RepathInterval = MaxRepathInterval;
		SetTarget(Target);
		return;
	}

	// Get current target
	ADungeonSingleTile* currTarget = DesiredPath.Last();
	ADungeonSingleTile* next = nullptr;
	// Scan current targets' connections to see if the target is contained in them
	for (int i = 0; i < 4; ++i)
		if (currTarget->CardinalConnections[i] == currTarget)
		{
			next = currTarget->CardinalConnections[i];
			break;
		}
	// Target is outside potential range, repath
	if (!next)
	{
		SetTarget(Target);
		return;
	}
	DesiredPath.Add(next);
}

void ADungeonCrawlerEnemy::AlterHealth(int amount)
{
	Health = FMath::Clamp(Health + amount, 0, 99);
	if (Health == 0)
	{
		if (CurrentTile)
			CurrentTile->OccupyingActor = nullptr;
		ReceiveOnDeath();
	}
	else
	{
		ReceiveTakeDamage();
	}
}

void ADungeonCrawlerEnemy::ClearTileAndSelf()
{
	if (Gamemode && !Gamemode->ToBeKilledEnemies.Contains(this))
		Gamemode->ToBeKilledEnemies.Add(this);
	Destroy();
}

// Called every frame
void ADungeonCrawlerEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}