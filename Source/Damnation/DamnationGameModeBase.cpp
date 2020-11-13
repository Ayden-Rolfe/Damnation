// Fill out your copyright notice in the Description page of Project Settings.

#include "DamnationGameModeBase.h"
#include "ProjectileInterface.h"

void ADamnationGameModeBase::InitDungeonMap(bool bSpawnPlayer)
{
	auto world = GetWorld();

	// Assign player to variable if spawning them
	if (bSpawnPlayer)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ActivePlayer = world->SpawnActor<ADungeonCrawlerPlayer>(PlayerActorType, spawnParams);
		ActivePlayer->SetGamemode(this);
	}

	BindColorMapEvents();
	// Map of the game world
	if (DungeonMap)
	{
		DungeonMap->SetGamemode(this);
		DungeonMap->GenerateFloor();
	}

	GenerateMinimap();

	// Spawn eye keys
	PlaceEyes(DungeonMap->GetRoom(DungeonMap->StarterRoomPosition + FVector2D(1, 0)));

	// Call BP logic once the game is prepared
	BeginGame();
}

void ADamnationGameModeBase::InitBossMap()
{
	auto world = GetWorld();

	// Assign player to vars
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActivePlayer = GetWorld()->SpawnActor<ADungeonCrawlerPlayer>(PlayerActorType, spawnParams);

	BindColorMapEvents();
	// Map of the boss room
	if (DungeonMap)
	{
		DungeonMap->SetGamemode(this);
		DungeonMap->GenerateBossRoom();
	}
}

void ADamnationGameModeBase::DoEnemyMovement()
{
	for (auto enemy : ActiveEnemies)
	{
		enemy->PerformMovement();
	}
	CleanupDeadEnemies();

	TArray<AActor*> projectilesToDelete;
	projectilesToDelete.Reserve(ActiveProjectiles.Num());

	for (auto projectile : ActiveProjectiles)
	{
		if (IProjectileInterface::Execute_ProjectileAction(projectile))
			projectilesToDelete.Add(projectile);
	}
	// Destroy projectiles that are set to be destroyed
	for (AActor* projectile : projectilesToDelete)
	{
		ActiveProjectiles.RemoveSwap(projectile);
		projectile->Destroy();
	}

	// Set last players' position
	LastTile = ActivePlayer->CurrentTile;
	PlayerAction();
}

void ADamnationGameModeBase::CleanupDeadEnemies()
{
	for (ADungeonCrawlerEnemy* enemy : ToBeKilledEnemies)
	{
		if (enemy)
		{
			ActiveEnemies.RemoveSwap(enemy);
		}
	}
	ToBeKilledEnemies.Empty();
}

void ADamnationGameModeBase::DoTormentorAction()
{
	if (ActiveTormentor)
	{
		ActiveTormentor->PerformMovement();
	}
}

TArray<ADungeonSingleTile*> ADamnationGameModeBase::CallPathfinder(ADungeonSingleTile* start, ADungeonSingleTile* end, int size, bool GoForClosest, bool respectOccupants)
{
	return DungeonMap->GeneratePath(start, end, size, GoForClosest, respectOccupants);
}

void ADamnationGameModeBase::SetPlayerLocation(ADungeonSingleTile* target)
{
	if (target)
	{
		target->OccupyingActor = ActivePlayer;
		ActivePlayer->CurrentTile = target;
		ActivePlayer->SetActorLocation(target->GetActorLocation());
	}
}

void ADamnationGameModeBase::AddTormentorLocation(ADungeonSingleTile* tile)
{
	TormentorSpawnLocations.Add(tile);
}

void ADamnationGameModeBase::AddEyeLocation(ADungeonRoomTileBase* room, FVector2D position)
{
	EyeSpawns.Add(TPair<FVector2D, ADungeonRoomTileBase*>(position, room));
}

void ADamnationGameModeBase::DespawnEnemies()
{
	TArray<unsigned int> DespawnList;
	DespawnList.Reserve(ActiveEnemies.Num());
	float squaredDespawnDistance = FMath::Square(DespawnDistance);
	for (int i = 0; i < ActiveEnemies.Num(); ++i)
	{
		float dist = FVector::DistSquared(ActivePlayer->GetActorLocation(), ActiveEnemies[i]->GetActorLocation());
		if (dist > squaredDespawnDistance)
		{
			DespawnList.Add(i);
		}
	}
	// Reverse list so we remove enemies top->bottom
	Algo::Reverse(DespawnList);
	for (auto i : DespawnList)
	{
		ActiveEnemies[i]->CurrentTile->OccupyingActor = nullptr;
		ActiveEnemies[i]->Destroy();
		ActiveEnemies.RemoveAt(i, 1, false);
	}
	ActiveEnemies.Shrink();
	// Any remaining enemies in the list are close enough to the player that despawning them would look weird
}

void ADamnationGameModeBase::GenerateMinimap_Implementation()
{
}

void ADamnationGameModeBase::PlaceEyes(ADungeonRoomTileBase* ReqRoom)
{
	ActiveEyeTiles.Reserve(EyeSpawns.Num());
	if (ReqRoom)
	{
		FVector2D spawn = EyeSpawns.FindByPredicate([ReqRoom](TPair<FVector2D, ADungeonRoomTileBase*> spawn) {return spawn.Value == ReqRoom; })->Key;
		ReqRoom->AddTileEntity(EyeActorType, spawn);
		// As this tile is now occupied with an unremovable object, set it as pathfinding-invalid
		auto tile = ReqRoom->GetTile(spawn);
		tile->PermitPathing(false);
		ActiveEyeTiles.Add(tile);
		EyeCount--;
		int idx = EyeSpawns.IndexOfByPredicate([ReqRoom](TPair<FVector2D, ADungeonRoomTileBase*> spawn) {return spawn.Value == ReqRoom; });
		if (idx != INDEX_NONE)
			EyeSpawns.RemoveAtSwap(idx);
	}

	TArray<FVector> EyePositions;
	EyePositions.Reserve(EyeSpawns.Num());
	float MinDistSquared = FMath::Square(EyeSpacingDistance);
	ShuffleArray(EyeSpawns);

	// Picks eye spawns at random that aren't too close to each other
	for (int e = EyeCount; e > 0; --e)
	{
		ADungeonSingleTile* tile = nullptr;
		// Iterate until we find a valid tile to spawn on
		while (!tile)
		{
			if (EyeSpawns.Num() == 0)
			{
				EyeCount = (EyeCount - e);
				break;
			}
			int idx = FMath::RandRange(0, EyeSpawns.Num() - 1);
			// If the eye spawn world position is too close to an existing eye location.
			FVector worldPosition = EyeSpawns[idx].Value->GetRoomTilePosition(EyeSpawns[idx].Key);
			bool valid = true;
			for (auto existingEyePos : EyePositions)
			{
				float dist = FVector::DistSquared(worldPosition, existingEyePos);
				if (dist < MinDistSquared)
				{
					valid = false;
					break;
				}
			}
			if (valid)
			{
				EyeSpawns[idx].Value->AddTileEntity(EyeActorType, EyeSpawns[idx].Key);
				// As this tile is now occupied with an unremovable object, set it as pathfinding-invalid
				tile = EyeSpawns[idx].Value->GetTile(EyeSpawns[idx].Key);
				tile->PermitPathing(false);
				ActiveEyeTiles.Add(tile);

				EyePositions.Add(worldPosition);
			}
			// Regardless of if it's valid or not, the index is to be removed.
			EyeSpawns.RemoveAtSwap(idx);
		}
	}
}
