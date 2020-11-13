// Fill out your copyright notice in the Description page of Project Settings.

#include "DungeonTormentor.h"
#include "DamnationGameModeBase.h"

// Sets default values
ADungeonTormentor::ADungeonTormentor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Gamemode = dynamic_cast<ADamnationGameModeBase*>(UGameplayStatics::GetGameMode(GetWorld()));
	if (Gamemode)
		Gamemode->ActiveTormentor = this;
}

// Called when the game starts or when spawned
void ADungeonTormentor::BeginPlay()
{
	Super::BeginPlay();
	
	Health = MaxHealth;
	ActionTime = MoveDuration;
}

// Called every frame
void ADungeonTormentor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// FVector desiredDir = (-ModelPosition->GetRelativeTransform().GetLocation()).GetSafeNormal();
	// ModelPosition->AddLocalOffset(desiredDir * DeltaTime);
}

void ADungeonTormentor::PerformMovement()
{
	if (TimeUntilActionPermitted > 0.0f || bIsDead)
		return;

	// Update oldpos to match current pos so lagged root doesn't wig out
	// Will be properly set later if necessary
	OldTransform = GetActorTransform();

	// See if we can spot the player in our attack ranges
	TArray<ADungeonSingleTile*> tileSet;
	bool playerInSlamRange = false;
	bool slamValid = GetSlamAttackTiles(Facing, tileSet);
	for (auto tile : tileSet)
		if (tile && dynamic_cast<ADungeonCrawlerPlayer*>(tile->OccupyingActor))
		{
			playerInSlamRange = true;
			break;
		}

	bool playerInSwipeRange = false;
	bool swipeValid = GetSwipeAttackTiles(Facing, tileSet);
	for (auto tile : tileSet)
		if (tile && dynamic_cast<ADungeonCrawlerPlayer*>(tile->OccupyingActor))
		{
			playerInSwipeRange = true;
			break;
		}

	bool playerInAttackRange = playerInSlamRange || playerInSwipeRange;
	if (playerInAttackRange)
	{
		if (playerInSlamRange && playerInSwipeRange)
		{
			if (!swipeValid)
				attackPrepared = ETormentorAttackType::SLAM;
			else
				attackPrepared = (ETormentorAttackType)FMath::RandRange(0, 1);
		}
		else if (playerInSlamRange) attackPrepared = ETormentorAttackType::SLAM;
		else attackPrepared = ETormentorAttackType::SWIPE;
		bIsAttacking = true;
	}

	if (bIsAttacking)
	{
		switch (attackPrepared)
		{
		case ETormentorAttackType::SWIPE:
			if (bIsAttackPrepared) ActionTime = SwipeAttackDuration;
			else ActionTime = SwipeAttackWindupDuration;
			AttackSwipe();
			break;
		case ETormentorAttackType::SLAM:
			if (bIsAttackPrepared) ActionTime = SlamAttackDuration;
			else ActionTime = SlamAttackWindupDuration;
			AttackSlam();
			break;
		default:
			break;
		}
	}
	else
	{
		ReceiveMoveAction();

		// Can see player, repath if necessary
		auto sightResult = SightCheck();
		ADungeonCrawlerPlayer* playerSight = dynamic_cast<ADungeonCrawlerPlayer*>(sightResult.Actor.Get());
		bCanSeePlayer = playerSight && sightResult.Distance ? true : false;
		ReceivePlayerSightCheck(bCanSeePlayer);

		// This is the first time we've spotted the player, do relevant first-spot actions
		if (bFirstPlayerSight && bCanSeePlayer)
		{
			ReceiveOnSpotPlayer();
			bFirstPlayerSight = false;
			ActionTime = SpotPlayerActionDuration;
		}
		else
		{
			// If we see the player but we're ignoring them, only repath if they are immediately reachable.
			if (bIsIgnoringPlayer && playerSight)
				bIsIgnoringPlayer = !SetTarget(playerSight->CurrentTile, false);

			// We can't see the player, stop ignoring them so we don't accidentally ignore them when we can reach them next time we see them
			else bIsIgnoringPlayer = false;

			// If player is visible, reachable & not where we saw them last, make a path to them.
			if (!bIsIgnoringPlayer && playerSight && playerSight->CurrentTile != LastPlayerSeenTile)
			{
				SetTarget(playerSight->CurrentTile);
				LastPlayerSeenTile = playerSight->CurrentTile;
			}

			if (DesiredPath.Num() > 0)
			{
				OnTormentorMovementAction.Broadcast();
				ADungeonSingleTile* nextTile = DesiredPath[0];
				// Check tiles forward of desired tile based on movement direction
				// Get direction of movement
				uint8 dir = 0;
				while (CurrentTile->CardinalConnections[dir] != nextTile) ++dir;

				// Check if our facing direction matches the next intended direction
				if (dir != (uint8)Facing)
				{
					RotateToDirection((ECardinal)dir);
					ActionTime = RotateDuration;
				}
				else
				{
					AActor* occupant = nullptr;
					// array of tiles that will be occupied on move if successful
					TArray<ADungeonSingleTile*> occupyTiles;
					GetTilesDirectional(dir, occupyTiles);

					for (auto occupyTile : occupyTiles)
					{
						if (occupyTile)
							if (occupyTile->OccupyingActor && occupyTile->OccupyingActor != this)
							{
								occupant = occupyTile->OccupyingActor;
								break;
							}
					}

					// LaggedRoot->SetRelativeRotation(FQuat(FRotator(0, (dir - 1) * 90.0f, 0)));

					// Something is in the way, do a swipe attack to get it out of the way
					if (occupant != nullptr)
					{
						if (!swipeValid)
							attackPrepared = ETormentorAttackType::SLAM;
						else
							attackPrepared = (ETormentorAttackType)FMath::RandRange(0, 1);
						bIsAttacking = true;
						// To prevent delay between detecting blocking entity & preparing the attack, immediately activate next action.
						ActionTime = 0.0f;
					}
					else
					{
						// Get location offset to displace model on actor location update
						FVector locationOffset = nextTile->GetActorLocation() - GetActorLocation();
						OldTransform = GetActorTransform();
						SetActorLocation(nextTile->GetActorLocation());

						SetTile(nextTile);

						//if (DesiredPath.Num() > 1)
						DesiredPath.RemoveAt(0, 1, false);

						ActionTime = MoveDuration;
					}
				}
			}
			else
			{
				OnTormentorPathEndReached.Broadcast();
				// Reached the end of our path; either because we have finished pathing to a random room, or because we can't reach the players' true position.
				if (playerSight)
				{
					// Get direction to try and face player
					// Directional vector to cardinal dir
					FVector playerPos = playerSight->GetActorLocation();
					FVector tormenPos = GetActorLocation();
					// + 360 to ensure positive, is modulo'd next anyway
					float dirVec = (FMath::RadiansToDegrees(FMath::Atan2(playerPos.Y - tormenPos.Y, playerPos.X - tormenPos.X))) + 360.0f;
					ECardinal dir = ECardinal((int((dirVec / 90.0f) + .5f)) % 4);

					if (Facing != dir)
					{
						RotateToDirection(dir);
						ActionTime = RotateDuration;
					}
					// We are facing the player, at the end of our path, but they are still out of reach from our attacks.
					// Leave them be, ignoring their movements until they go to a spot we can actually reach them at.
					else
					{
						ReceiveOnCannotReachTarget();
						bIsIgnoringPlayer = true;
						ActionTime = SpotPlayerActionDuration;
						SetTarget(Gamemode->DungeonMap->GetRoomRandom()->GetTileRandom());
					}
				}
				// Lost sight of the player & we've reached the end of our path, simply try to go somewhere else
				else
				{
					SetTarget(Gamemode->DungeonMap->GetRoomRandom()->GetTileRandom());
				}
			}
		}
	}
	TimeUntilActionPermitted = ActionTime;

	ReceiveOnMove(OldTransform, GetActorTransform());
}

void ADungeonTormentor::RotateToDirection(const ECardinal& dir)
{
	// Get difference to rotate the transform by
	uint8 diff = (((uint8)dir - (uint8)Facing) + 4) % 4;
	// Clamp rotation to 90 degrees per action max.
	if (diff == 2)
		diff = FMath::RandBool() ? 1 : 3;
	ECardinal newDir = ECardinal(((uint8)Facing + diff) % 4);

	// Relative direction we're about to make. from 1..3 to -1..1
	int dirRelative = int(diff) - 2;
	OnTormentorRotate.Broadcast(dirRelative);

	float diffVal = (float)(diff * 90);

	// We have to turn to face the new direction
	ReceiveOnRotate(Facing, newDir);

	// Get new rotation for transform & apply
	FRotator deltaRot = FRotator::ZeroRotator;
	deltaRot.Add(0, diffVal, 0);
	AddActorWorldRotation(deltaRot);
	Facing = newDir;
}

bool ADungeonTormentor::AlterHealth(int val)
{
	if (bIsVulnerable)
	{
		Health = FMath::Clamp(Health + val, 0, MaxHealth);
		ReceiveOnHurt();
		if (Health == 0)
		{
			bIsDead = true;
			ReceiveOnDeath();
			OnTormentorDeath.Broadcast();
			// Clear tiles occupying
			if (CurrentTile)
			{
				// Clear current adjacent tiles
				TArray<ADungeonSingleTile*> currentNearTiles;
				CurrentTile->GetSurroundingTiles(currentNearTiles);
				for (ADungeonSingleTile* nearTile : currentNearTiles)
					if (nearTile) nearTile->OccupyingActor = nullptr;
				CurrentTile->OccupyingActor = nullptr;
			}
		}
	}
	return bIsVulnerable;
}

bool ADungeonTormentor::GetTilesDirectional(int direction, TArray<ADungeonSingleTile*>& tileList)
{
	direction = direction % 4;
	tileList.Empty(3);
	// Middle tile in forward dir
	// Jumps forward 1 to account for 3x3 size of tormentor
	auto mid = CurrentTile->CardinalConnections[direction]->CardinalConnections[direction];
	// x + 3 % 4 is effectively -1 for getting direction difference without out of range index error possibility
	if (mid)
	{
		tileList.Add(mid->CardinalConnections[(direction + 3) % 4]);
		tileList.Add(mid);
		tileList.Add(mid->CardinalConnections[(direction + 1) % 4]);
	}

	for (auto tile : tileList)
		if (!tile)
			return false;
	return true;
}

bool ADungeonTormentor::GetSwipeAttackTiles(int direction, TArray<ADungeonSingleTile*>& tileList)
{
	direction = direction % 4;
	tileList.Empty(10);
	// Middle tile in forward dir
	// Jumps forward 1 to account for 3x3 size of tormentor
	auto mid = CurrentTile->CardinalConnections[direction]->CardinalConnections[direction];
	// x + 3 % 4 is effectively -1 for getting direction difference without out of range index error possibility
	if (mid)
	{
		// Leftmost tiles
		auto leftT = mid->CardinalConnections[(direction + 3) % 4];
		if (leftT)
			tileList.Add(leftT->CardinalConnections[(direction + 3) % 4]);
		else tileList.Add(nullptr);
		tileList.Add(leftT);

		tileList.Add(mid);

		// Rightmost tiles
		auto rightT = mid->CardinalConnections[(direction + 1) % 4];
		if (rightT)
			tileList.Add(rightT->CardinalConnections[(direction + 1) % 4]);
		else tileList.Add(nullptr);
		tileList.Add(rightT);


		// get the tile 1 ahead of each stored tile to make swipe attack 2x5
		for (int i = 0; i < 5; ++i)
		{
			if (tileList[i])
				tileList.Add(tileList[i]->CardinalConnections[direction]);
			else tileList.Add(nullptr);
		}
	}

	for (auto tile : tileList)
		if (!tile)
			return false;
	return true;
}

bool ADungeonTormentor::GetSlamAttackTiles(int direction, TArray<ADungeonSingleTile*>& tileList)
{
	direction = direction % 4;
	// Jumps ahead 3 tiles (from middle tormentor, to tormentor edge, to in front of tormentor, to 1 tile in front of tormentor)
	ADungeonSingleTile* mid = CurrentTile->CardinalConnections[direction];
	for (int i = 0; i < 2; ++i)
	{
		mid = mid->CardinalConnections[direction];
		if (!mid)
			return false;
	}
	mid->GetSurroundingTiles(tileList);
	tileList.Add(mid);
	return true;
}

void ADungeonTormentor::AttackSwipe_Implementation()
{
}

void ADungeonTormentor::AttackSlam_Implementation()
{
}

bool ADungeonTormentor::SetTarget(ADungeonSingleTile* Target, bool GoForClosest)
{
	TArray<ADungeonSingleTile*> path;
	path = Gamemode->CallPathfinder(CurrentTile, Target, 3, GoForClosest);
	if (path.Num() == 0)
		return false;
	else DesiredPath = path;
	return true;
}

void ADungeonTormentor::SetTile(ADungeonSingleTile* tile)
{
	if (CurrentTile)
	{
		// Clear current adjacent tiles
		TArray<ADungeonSingleTile*> currentNearTiles;
		CurrentTile->GetSurroundingTiles(currentNearTiles);
		for (ADungeonSingleTile* nearTile : currentNearTiles)
			if (nearTile) nearTile->OccupyingActor = nullptr;
	}

	Super::SetTile(tile);

	// Sets adjacent tiles occupying actor to this too
	// also trigger any events on those tiles
	TArray<ADungeonSingleTile*> nearbyTiles;
	CurrentTile->GetSurroundingTiles(nearbyTiles);
	for (ADungeonSingleTile* nearTile : nearbyTiles)
		if (nearTile)
		{
			nearTile->OccupyingActor = this;
			nearTile->TileEvent.Broadcast(this, nearTile);
		}
}