// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonRoomTileBase.h"
#include "DungeonMacroGrid.h"
#include "DamnationGameModeBase.h"

// Sets default values
ADungeonRoomTileBase::ADungeonRoomTileBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ValidCardinals.Init(true, 4);

	CurrentRespawnInterval = FMath::FRandRange(0.0f, RespawnInterval);
}

// Called when the game starts or when spawned
void ADungeonRoomTileBase::BeginPlay()
{
	Super::BeginPlay();
}

void ADungeonRoomTileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	FlatArraySize = GridEdgeLength * GridEdgeLength;

	// Allocate space to be ready for additions
	TileGridFlatArray.Init(nullptr, FlatArraySize);
}

// Called every frame
void ADungeonRoomTileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentRespawnInterval -= DeltaTime;

	if (CurrentRespawnInterval <= 0.0f)
	{
		// Shuffle spawn array
		ShuffleArray(SpawnDataContainer.DataArray);
		auto gm = dynamic_cast<ADamnationGameModeBase*>(UGameplayStatics::GetGameMode(this));
		if (gm && gm->ActivePlayer)
		{
			AActor* player = gm->ActivePlayer;
			for (auto spawnData : SpawnDataContainer.DataArray)
			{
				if (RespawnCurrentCount <= 0)
					break;
				// Ensure the spawn is far enough away from the player, & that the spawn tile doesn't have something there already.
				float PlayerSpawnDistance = FVector::DistSquared(GetRoomTilePosition(spawnData.Spawn), player->GetActorLocation());
				if (PlayerSpawnDistance >= FMath::Square(RespawnMinDistance) && !GetTile(spawnData.Spawn)->OccupyingActor)
				{
					SpawnEnemy(spawnData);
					--RespawnCurrentCount;
				}
			}
		}

		// If enemies are still waiting to respawn, use a shorter delay between respawn checks.
		if (RespawnCurrentCount > 0)
			CurrentRespawnInterval = RespawnAwaitingInterval;
		else CurrentRespawnInterval = RespawnInterval;
	}
}

ADungeonSingleTile* ADungeonRoomTileBase::ForceTileDisconnect(ECardinal direction, ADungeonSingleTile* targetTile)
{
	if (targetTile)
	{
		// Get accessor from ECardinal
		uint8 accessor = (uint8)direction;
		ADungeonSingleTile* hold = targetTile->CardinalConnections[accessor];
		targetTile->CardinalConnections[accessor] = nullptr;
		return hold;
	}
	else
	{
		return nullptr;
	}
}

void ADungeonRoomTileBase::ForceTileConnect(ECardinal direction, ADungeonSingleTile* targetTile, ADungeonSingleTile* linkingTile)
{
	if (targetTile)
	{
		targetTile->CardinalConnections[(uint8)direction] = linkingTile;
	}
}

void ADungeonRoomTileBase::SpawnEnemy(FEnemySpawnData& data)
{
	ADungeonCrawlerEnemy* enemy = dynamic_cast<ADungeonCrawlerEnemy*>(AddTileEntity(data.Type, data.Spawn));
	enemy->OriginRoom = this;
}

void ADungeonRoomTileBase::SpawnEnemies(FEnemySpawnDataArrayContainer spawnInfo)
{
	SpawnDataContainer = spawnInfo;
	int count = FMath::RandRange(EnemySpawnsMinimum, EnemySpawnsMaximum);
	TArray<FEnemySpawnData>& dataRef = SpawnDataContainer.DataArray;
	ShuffleArray(dataRef);
	for (int i = 0; i < count; ++i)
	{
		SpawnEnemy(dataRef[i]);
	}
}

ADungeonSingleTile* ADungeonRoomTileBase::AddTile(FVector2D position)
{
	ADungeonSingleTile* tileAtPosition = GetTile(position);
	if (!tileAtPosition)
	{
		FVector worldPos = GetRoomTilePosition(position);
		tileAtPosition = GWorld->SpawnActor<ADungeonSingleTile>(ADungeonSingleTile::StaticClass(), FTransform(worldPos));

		ADungeonSingleTile* currentCheck = nullptr;
		// Scan cardinals to add new connections + connect to this

		// North of newTile
		currentCheck = GetTile(position + FVector2D(0, 1));
		if (currentCheck) CheckSetCardinal(0, 2, tileAtPosition, currentCheck);

		// East of newTile
		currentCheck = GetTile(position + FVector2D(1, 0));
		if (currentCheck) CheckSetCardinal(1, 3, tileAtPosition, currentCheck);

		// South of newTile
		currentCheck = GetTile(position + FVector2D(-1, 0));
		if (currentCheck) CheckSetCardinal(2, 0, tileAtPosition, currentCheck);

		// West of newTile
		currentCheck = GetTile(position + FVector2D(0, -1));
		if (currentCheck) CheckSetCardinal(3, 1, tileAtPosition, currentCheck);

		// Assign value in flatarray
		int index = GridToFlatIndex(position);
		TileGridFlatArray[index] = tileAtPosition;
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("AddTile attempted to place tile at position where a room already exists. Returning existing room."))
	}
	return tileAtPosition;
}

ADungeonTileOccupant* ADungeonRoomTileBase::AddTileEntity(TSubclassOf<ADungeonTileOccupant> type, FVector2D position)
{
	if (type)
	{
		auto tile = GetTile(position);
		if (tile->OccupyingActor)
		{
			return nullptr;
		}
		auto occupant = GWorld->SpawnActor<ADungeonTileOccupant>(type, GetRoomTilePosition(position), FRotator::ZeroRotator);
		occupant->SetTile(tile);
		return occupant;
	}
	return nullptr;
}

FVector2D ADungeonRoomTileBase::FlatToGridIndex(int flatIndex)
{
	FVector2D out;
	out.X = flatIndex % GridEdgeLength;
	out.Y = flatIndex / GridEdgeLength;
	return out;
}

int ADungeonRoomTileBase::GridToFlatIndex(FVector2D position)
{
	int horizontalOffset = (int)position.Y * GridEdgeLength;
	int index = horizontalOffset + position.X;
	return index;
}

ADungeonSingleTile* ADungeonRoomTileBase::GetTile(FVector2D position)
{
	if ((int)position.X >= GridEdgeLength || (int)position.Y >= GridEdgeLength || position.X < 0 || position.Y < 0)
		return nullptr;
	else return TileGridFlatArray[GridToFlatIndex(position)];
}

ADungeonSingleTile* ADungeonRoomTileBase::GetTileRandom(int size)
{
	// Make array of all Indices
	TArray<uint32> possibleIndices;
	possibleIndices.Init(0, RoomTileCount);
	for (int i = 0; i < RoomTileCount; ++i)
		possibleIndices[i] = i;
	// Shuffle array so indices in array order 0..Num() act as a "random shuffle"
	ShuffleArray<uint32>(possibleIndices);
	// Iterate through indices from 0 to end and return the first valid result
	ADungeonSingleTile* randTile;
	for (int i = 0; i < RoomTileCount; ++i)
	{
		randTile = GetTileByIndex(possibleIndices[i]);
		if (randTile && randTile->availableSpace >= size)
			return randTile;
	}
	// This room has no valid tiles, return nullptr 
	return nullptr;
}

FVector ADungeonRoomTileBase::GetRoomTilePosition(FVector2D position)
{
	return FVector((position.X * TileSeparation) + (TileSeparation / 2), (position.Y * TileSeparation) + (TileSeparation / 2), 0.0f) + GetActorLocation();
}

ADungeonSingleTile* ADungeonRoomTileBase::GetTileByIndex(int index)
{
	if (index < 0 || index >= FlatArraySize)
		return nullptr;
	else return TileGridFlatArray[index];
}

void swap(FColor* a, FColor* b)
{
	FColor temp = *a;
	*a = *b;
	*b = temp;
}

void ADungeonRoomTileBase::LoadTextureToMap()
{
	const TMap<FColor, ADamnationGameModeBase::FTileColorSpawn>& colorMap = MacroGrid->GetGamemode()->ColorDelegateMap;

	if (!MapTexture) return;
	if (MapTexture->GetSizeX() > GridEdgeLength || MapTexture->GetSizeY() > GridEdgeLength) return;

	const FColor* MapDataPixels = static_cast<const FColor*>(MapTexture->PlatformData->Mips[0].BulkData.LockReadOnly());

	FColor FlippedTextureData[GridEdgeLength][GridEdgeLength];
	for (int32 x = 0; x < GridEdgeLength; ++x)
		for (int32 y = 0; y < GridEdgeLength; ++y)
			FlippedTextureData[x][y] = MapDataPixels[x * MapTexture->GetSizeX() + y];

	MapTexture->PlatformData->Mips[0].BulkData.Unlock();

	for (int i = 0; i < GridEdgeLength; ++i)
	{
		int start = 0;
		int end = GridEdgeLength - 1;
		while (start < end)
		{
			swap(&FlippedTextureData[start][i], &FlippedTextureData[end][i]);
			start++;
			end--;
		}
	}

	for (int32 x = 0; x < GridEdgeLength; ++x)
		for (int32 y = 0; y < GridEdgeLength; ++y)
		{
			FColor pixelColor = FlippedTextureData[x][y];
			auto test = colorMap.Find(pixelColor);
			if (test)
				test->Execute(this, FVector2D(x, y));
		}

	// Map has been loaded, call map loaded event for blueprint visual implementations
	OnMapLoad();
}

void ADungeonRoomTileBase::AssignSizes()
{
	for (auto tile : TileGridFlatArray)
		if (tile)
			tile->availableSpace = (tile->CheckSurroundingTiles()) ? 3 : 1;
}

void ADungeonRoomTileBase::DestroyRoom()
{
	for (auto tile : TileGridFlatArray)
		if (tile)
			tile->Destroy();
	Destroy();
}

