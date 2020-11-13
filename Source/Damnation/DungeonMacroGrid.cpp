// Fill out your copyright notice in the Description page of Project Settings.

#include "DungeonMacroGrid.h"
#include "DamnationGameModeBase.h"

// Sets default values
ADungeonMacroGrid::ADungeonMacroGrid()
{
	DirectionalBiases.Init(1.0, 4);

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MapOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("Handle"));
	RootComponent = MapOrigin;
}

float ADungeonMacroGrid::GetFill()
{
	float cFill = maxFillChance;
	maxFillChance = FMath::Max(maxFillChance - fillChanceVelocity, minFillChance);
	return cFill;
}

// Called when the game starts or when spawned
void ADungeonMacroGrid::BeginPlay()
{
	Super::BeginPlay();
}

void ADungeonMacroGrid::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Gamemode = dynamic_cast<ADamnationGameModeBase*>(UGameplayStatics::GetGameMode(GetWorld()));

	// Define map variables (cannot be done in constructor due to operation order for editor variables)
	ArrayWidth = MapMaxWidth;
	ArrayHeight = MapMaxHeight;

	FlatArraySize = ArrayWidth * ArrayHeight;

	// Allocate space to be ready for additions
	RoomGridFlatArray.Init(nullptr, FlatArraySize);
}

inline void ADungeonMacroGrid::ConnectNorthRooms(ADungeonRoomTileBase* roomA, ADungeonRoomTileBase* roomB)
{
	for (int y = 0; y < 15; ++y)
		if (roomA->GetTile(FVector2D(14, y)) && roomB->GetTile(FVector2D(0, y)))
		{
			roomA->GetTile(FVector2D(14, y))->CardinalConnections[0] = roomB->GetTile(FVector2D(0, y));
			roomB->GetTile(FVector2D(0, y))->CardinalConnections[2] = roomA->GetTile(FVector2D(14, y));
		}
}

inline void ADungeonMacroGrid::ConnectEastRooms(ADungeonRoomTileBase* roomA, ADungeonRoomTileBase* roomB)
{
	for (int x = 0; x < 15; ++x)
		if (roomA->GetTile(FVector2D(x, 14)) && roomB->GetTile(FVector2D(x, 0)))
		{
			roomA->GetTile(FVector2D(x, 14))->CardinalConnections[1] = roomB->GetTile(FVector2D(x, 0));
			roomB->GetTile(FVector2D(x, 0))->CardinalConnections[3] = roomA->GetTile(FVector2D(x, 14));
		}
}

inline void ADungeonMacroGrid::ConnectSouthRooms(ADungeonRoomTileBase* roomA, ADungeonRoomTileBase* roomB)
{
	for (int y = 0; y < 15; ++y)
		if (roomA->GetTile(FVector2D(0, y)) && roomB->GetTile(FVector2D(14, y)))
		{
			roomA->GetTile(FVector2D(0, y))->CardinalConnections[2] = roomB->GetTile(FVector2D(14, y));
			roomB->GetTile(FVector2D(14, y))->CardinalConnections[0] = roomA->GetTile(FVector2D(0, y));
		}
}

inline void ADungeonMacroGrid::ConnectWestRooms(ADungeonRoomTileBase* roomA, ADungeonRoomTileBase* roomB)
{
	for (int x = 0; x < 15; ++x)
		if (roomA->GetTile(FVector2D(x, 0)) && roomB->GetTile(FVector2D(x, 14)))
		{
			roomA->GetTile(FVector2D(x, 0))->CardinalConnections[3] = roomB->GetTile(FVector2D(x, 14));
			roomB->GetTile(FVector2D(x, 14))->CardinalConnections[1] = roomA->GetTile(FVector2D(x, 0));
		}
}

void ADungeonMacroGrid::SetPlayerLocation(ADungeonSingleTile* tile)
{
	Gamemode->SetPlayerLocation(tile);
}

void ADungeonMacroGrid::AddTormentorLocation(ADungeonSingleTile* tile)
{
	Gamemode->AddTormentorLocation(tile);
}

FVector ADungeonMacroGrid::ConvertPosition(FVector2D position)
{
	float size = (ADungeonRoomTileBase::GridEdgeLength * ADungeonRoomTileBase::TileSeparation);
	FVector gridPos(position.X * size, position.Y * size, 0.0f);
	return gridPos + FVector(size / 2, size / 2, 0.0f);
}

ADungeonRoomTileBase* ADungeonMacroGrid::AddRoom(FVector2D position, TSubclassOf<ADungeonRoomTileBase> roomType)
{
	// int index = GridToFlatIndex(position);
	ADungeonRoomTileBase* room = GetRoom(position);
	if (room)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddRoom: Room already at position specified."));
	}
	else
	{
		FTransform transform(GetActorLocation() + (UKismetMathLibrary::Conv_Vector2DToVector(position)) * ADungeonRoomTileBase::RoomPositionScalar);
		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		room = GetWorld()->SpawnActor<ADungeonRoomTileBase>(roomType, transform, spawnParams);
		room->SetMacroGrid(this);

		RoomGridFlatArray[GridToFlatIndex(position)] = room;

		room->LoadTextureToMap();

		// Scan valid cardinals for rooms to see if they can be connected to
		ADungeonRoomTileBase* adjRoom = nullptr;

		if (room->ValidCardinals[0])
		{
			adjRoom = GetRoom(position + FVector2D(1, 0));
			if (adjRoom && adjRoom->ValidCardinals[2])
			{
				ConnectNorthRooms(room, adjRoom);
				adjRoom->AssignSizes();
			}
		}
		if (room->ValidCardinals[1])
		{
			adjRoom = GetRoom(position + FVector2D(0, 1));
			if (adjRoom && adjRoom->ValidCardinals[3])
			{
				ConnectEastRooms(room, adjRoom);				
				adjRoom->AssignSizes();
			}
		}
		if (room->ValidCardinals[2])
		{
			adjRoom = GetRoom(position + FVector2D(-1, 0));
			if (adjRoom && adjRoom->ValidCardinals[0])
			{
				ConnectSouthRooms(room, adjRoom);
				adjRoom->AssignSizes();
			}
		}
		if (room->ValidCardinals[3])
		{
			adjRoom = GetRoom(position + FVector2D(0, -1));
			if (adjRoom && adjRoom->ValidCardinals[1])
			{
				ConnectWestRooms(room, adjRoom);
				adjRoom->AssignSizes();
			}
		}
		room->AssignSizes();
		room->OnMapFinalization();
	}
	return room;
}

ADungeonRoomTileBase* ADungeonMacroGrid::GetRoom(FVector2D position)
{
	if (!IsValidSpace(position))
		return nullptr;
	else return RoomGridFlatArray[GridToFlatIndex(position)];
}

ADungeonRoomTileBase* ADungeonMacroGrid::GetRoomByIndex(int index)
{
	if (index < 0 || index >= FlatArraySize)
		return nullptr;
	else return RoomGridFlatArray[index];
}

ADungeonRoomTileBase* ADungeonMacroGrid::GetRoomByWorldPosition(FVector position)
{
	FVector2D roomPosition = WorldToRoomPosition(position);
	return GetRoom(roomPosition);
}

ADungeonRoomTileBase* ADungeonMacroGrid::GetRoomRandom()
{
	// Make array of all Indices
	TArray<uint32> possibleIndices;
	uint32 roomCount = RoomGridFlatArray.Num();
	possibleIndices.Init(0, FlatArraySize);
	for (int i = 0; i < FlatArraySize; ++i)
		possibleIndices[i] = i;
	// Shuffle array so indices in array order 0..Num() act as a "random shuffle"
	ShuffleArray<uint32>(possibleIndices);
	// Iterate through indices from 0 to end and return the first valid result
	ADungeonRoomTileBase* randRoom;
	for (int i = 0; i < FlatArraySize; ++i)
	{
		randRoom = GetRoomByIndex(possibleIndices[i]);
		if (randRoom)
			return randRoom;
	}
	return nullptr;
}

bool ADungeonMacroGrid::IsValidSpace(FVector2D position)
{
	return !((int)position.X >= ArrayHeight || (int)position.Y >= ArrayWidth || position.X < 0 || position.Y < 0);
}

TArray<ADungeonSingleTile*> ADungeonMacroGrid::GeneratePath(ADungeonSingleTile* start, ADungeonSingleTile* end, int actorSize, bool getClosest, bool respectOccupants)
{
	// Return empty if start == end, standing on desired tile
	if (start == end)
		return TArray<ADungeonSingleTile*>();

	TArray<ADungeonSingleTile*> DesiredPath;

	TArray<ADungeonSingleTile*> AS_openTileList;
	TSet<ADungeonSingleTile*> AS_closedTileList;

	AS_openTileList.Add(start);
	ADungeonSingleTile* current = AS_openTileList[0];
	current->hCost = current->GetSquaredDistanceTo(end);
	// Store closest tile to target every iteration incase pathfinding fails
	ADungeonSingleTile* closest = current;

	while (AS_openTileList.Num() > 0)
	{
		current = AS_openTileList[0];
		if (current->GetSquaredDistanceTo(end) < closest->hCost) closest = current;
		AS_openTileList.RemoveAt(0);
		AS_closedTileList.Add(current);
		if (current == end)
		{
			DesiredPath.Empty();
			DesiredPath.Add(current);
			while (current->parent != start && current != start)
			{
				DesiredPath.Add(current->parent);
				current = current->parent;
			}
			DesiredPath.Shrink();
			Algo::Reverse<TArray<ADungeonSingleTile*>>(DesiredPath);
			return DesiredPath;
		}
		for (auto connection : current->CardinalConnections)
		{
			// IF
			// Connection exists, the closed list has the connection already, there's not enough space for the pather, or if the tile is set to be ignored
			if (!connection ||
				AS_closedTileList.Contains(connection) ||
				connection->availableSpace < actorSize ||
				connection->bPathingIgnore)
				continue;
			float sqrDistance = current->GetSquaredDistanceTo(end);
			// If the tile is occupied, increase the cost to encourage routing around obstacles.
			if (respectOccupants && connection->OccupyingActor && connection->OccupyingActor != Gamemode->ActivePlayer)
				sqrDistance *= 1.1;

			float moveCost = current->gCost + sqrDistance;
			if (moveCost < connection->gCost || !AS_openTileList.Contains(connection))
			{
				connection->gCost = moveCost;
				connection->hCost = sqrDistance;
				connection->parent = current;
				if (!AS_openTileList.Contains(connection))
					AS_openTileList.Add(connection);
			}
		}
		// Sort list by f score
		AS_openTileList.Sort([](const ADungeonSingleTile& LHS, const ADungeonSingleTile& RHS) {return LHS.fCost() < RHS.fCost(); });
	}
	// Cannot reach the desired position, pathfind to the closest valid position instead if desired.
	if (getClosest)
		return GeneratePath(start, closest, actorSize);
	// Return empty array if closest valid isn't desired.
	else return TArray<ADungeonSingleTile*>();
}

FVector2D ADungeonMacroGrid::FlatToGridIndex(int index)
{
	FVector2D out;
	out.X = index % ArrayWidth;
	out.Y = index / ArrayHeight;
	return out;
}

int ADungeonMacroGrid::GridToFlatIndex(FVector2D position)
{
	int horizontalOffset = (int)position.Y * ArrayHeight;
	int index = horizontalOffset + position.X;
	return index;
}

FVector2D ADungeonMacroGrid::WorldToRoomPosition(FVector position)
{
	return FVector2D
	(
		FMath::FloorToFloat(position.X / ADungeonRoomTileBase::RoomPositionScalar),
		FMath::FloorToFloat(position.Y / ADungeonRoomTileBase::RoomPositionScalar)
	);
}

FVector2D ADungeonMacroGrid::WorldToTilePosition(FVector position)
{
	return FVector2D
	(
		FMath::FloorToFloat(FMath::Fmod(position.X, ADungeonRoomTileBase::RoomPositionScalar) / ADungeonRoomTileBase::TileSeparation),
		FMath::FloorToFloat(FMath::Fmod(position.Y, ADungeonRoomTileBase::RoomPositionScalar) / ADungeonRoomTileBase::TileSeparation)
	);
}

ADungeonSingleTile* ADungeonMacroGrid::GetTileAtWorldPosition(FVector position)
{
	FVector2D roomPosition = WorldToRoomPosition(position);
	FVector2D tilePosition = WorldToTilePosition(position);

	auto room = GetRoom(roomPosition);
	if (room)
		return GetRoom(roomPosition)->GetTile(tilePosition);
	else return nullptr;
}

// More logical map generation system taking connectors into account
void ADungeonMacroGrid::GenerateFloor()
{
	// Jump table for positions
	static const FVector2D jumpDirTable[4] =
	{
		FVector2D(1, 0),
		FVector2D(0, 1),
		FVector2D(-1, 0),
		FVector2D(0, -1)
	};
	// Typedefs for ease of reading
	// TConPair == direction & room position, representing a connector
	// TConPairChance == TConPair + float, is TConPair with chance representing potential to spawn a new room there
	typedef TPair<int, FVector2D> TConPair;
	typedef TPair<TConPair, float> TConPairChance;

	// List of rooms with with the # of times they're used to encourage variation in spawned rooms
	typedef TPair<TSubclassOf<ADungeonRoomTileBase>, int> TRoomUses;
	TArray<TRoomUses> RoomListUses;
	RoomListUses.Init(TRoomUses(nullptr, 0), RoomList.Num());
	for (int i = 0; i < RoomList.Num(); ++i)
		RoomListUses[i] = TRoomUses(RoomList[i], 0);

	AddRoom(StarterRoomPosition, StarterRoom);
	AddRoom(StarterRoomPosition + FVector2D(1, 0), TutorialRoom);
	FVector2D MuralRoomPosition = EscapeRoomPosition - FVector2D(1, 0);
	AddRoom(EscapeRoomPosition, EscapeRoom);
	AddRoom(MuralRoomPosition, MuralRoom);

	TArray<TConPair> OpenConnectors;

	// Connectors to starting split & mural
	for (int i = 0; i < 3; ++i)
	{
		OpenConnectors.Add(TConPair((i + 1) % 4, MuralRoomPosition));
	}

	// Do Bresenham's Line Algorithm to make guaranteed path between start & end positions
	{
		// Find all rooms that are 4-way to put in list
		TArray<TRoomUses> validRooms;
		for (TRoomUses roomUses : RoomListUses)
		{
			bool roomValid = true;
			for (int i = 0; i < 4; ++i)
				if (!roomUses.Key.GetDefaultObject()->ValidCardinals[i])
				{
					roomValid = false;
					break;
				}
			if (roomValid)
				validRooms.Add(roomUses);
		}

		int x0 = StarterRoomPosition.X;
		int y0 = StarterRoomPosition.Y;
		int x1 = MuralRoomPosition.X;
		int y1 = MuralRoomPosition.Y;

		int mNew = 2 * (y1 - y0);
		int slopeError = mNew - (x1 - x0);

		for (int x = x0, y = y0; x <= x1; ++x)
		{
			// Ensure room used is likely to be unique
			validRooms[0].Value++;
			ShuffleArray<TRoomUses>(validRooms);
			validRooms.Sort([](const TRoomUses& LHS, const TRoomUses& RHS) {return LHS.Value < RHS.Value; });
			// Add the room to the position
			FVector2D bresPos = FVector2D(x, y);
			if ((AddRoom(bresPos, validRooms[0].Key)->GetClass() != StarterRoom.Get()))
				for (int i = 0; i < 4; ++i) OpenConnectors.Add(TConPair((i + 3) % 4, bresPos));

			// Algorithm-relevant
			slopeError += mNew;
			if (slopeError >= 0)
			{
				++y;
				slopeError -= 2 * (x1 - x0);
			}
		}
	}

	FVector2D current;

	while (OpenConnectors.Num() > 0)
	{
		int idx = FMath::RandRange(0, OpenConnectors.Num() - 1);
		TConPair connector = OpenConnectors[idx];
		FVector2D start = connector.Value;
		current = jumpDirTable[connector.Key] + connector.Value;
		auto room = GetRoom(current);
		if (!IsValidSpace(current))
		{
			UE_LOG(LogTemp, Error, TEXT("Connector leading to outside map boundaries; typically the result of the map being smaller than the mural/escape/start room positions. Increase map size or move the offending room."))
		}
		else if (!room)
		{
			// Tpair: Holds TConPair & float determining chance of connections in new room (0..1 range of probability)
			TArray<TConPairChance, TFixedAllocator<4>> newConnectorChances;
			// Check all adjacent rooms to the new room spot that aren't what we just came from
			for (int i = 0; i < 4; ++i)
			{
				float chance;
				FVector2D pos = jumpDirTable[i] + current;
				// If it cannot support a tile, make it guaranteed failure
				if (!IsValidSpace(pos)) chance = -1.0f;
				// Original room must be connected, so ensure it with guaranteed chance
				else if (pos == start) chance = 1.0f;
				else
				{
					TConPair nextConnector(i, pos);
					ADungeonRoomTileBase* nextRoom = GetRoom(pos);
					if (nextRoom)
					{
						// If there is a connector in room to current, make it a guaranteed connector
						// Else don't allow it to be a connector at all
						chance = nextRoom->ValidCardinals[(i + 2) % 4] ? 1.0f : -1.0f;
					}
					// If there's no existing room but can support a tile, make it a random chance of being a connector accounting for bias
					else
					{
						int relativeDir = ((i - connector.Key) + 4) % 4;

						chance = GetFill() * DirectionalBiases[relativeDir];
					}
				}
				newConnectorChances.Add(TConPairChance(TConPair(i, current), chance));
			}
			// Take chance array and make decisions on necessary connectors
			TArray<bool, TFixedAllocator<4>> newConnectors;
			newConnectors.Init(false, 4);
			for (int i = 0; i < 4; ++i)
			{
				// Position that the new connector is leading to
				FVector2D newConPos = FVector2D(newConnectorChances[i].Key.Value) + jumpDirTable[newConnectorChances[i].Key.Key];
				// Potential connection chance
				newConnectors[i] = newConnectorChances[i].Value >= FMath::FRand();
			}
			TArray<TRoomUses> validRooms;
			validRooms.Empty(RoomList.Num());
			// Find all rooms that meet the criteria in the room list
			for (TRoomUses roomUses : RoomListUses)
			{
				bool roomValid = true;
				for (int i = 0; i < 4; ++i)
					if (newConnectors[i] != roomUses.Key.GetDefaultObject()->ValidCardinals[i])
					{
						roomValid = false;
						break;
					}
				if (roomValid)
					validRooms.Add(roomUses);
			}
			// Shuffle, then sort by use count
			ShuffleArray<TRoomUses>(validRooms);
			validRooms.Sort([](const TRoomUses& LHS, const TRoomUses& RHS) {return LHS.Value < RHS.Value; });
			// Pick one of the rooms to place down, if any can be placed
			if (validRooms.Num() > 0 && IsValidSpace(current))
			{
				auto roomType = validRooms[0].Key;
				auto addedRoom = AddRoom(current, roomType);
				// Increment uses on roomlist to make more unlikely to pick
				int roomID = RoomListUses.Find(validRooms[0]);
				RoomListUses[roomID].Value++;
				// Add connectors of new room to list, except to previous room
				for (int i = 0; i < 4; ++i)
				{
					if (jumpDirTable[i] + current == start)
						continue;
					if (addedRoom->ValidCardinals[i])
						OpenConnectors.Add(TConPair(i, current));
				}
			}
		}
		// Remove the connector we just sorted out
		OpenConnectors.RemoveAtSwap(idx, 1, false);
		// Shuffle just for variation
		// ShuffleArray<TConPair>(OpenConnectors);
	}
	// DEBUG_GenerateNoiseMap();
}

void ADungeonMacroGrid::DestroyGeneration()
{
	for (int i = 0; i < RoomGridFlatArray.Num(); ++i)
	{
		auto room = RoomGridFlatArray[i];
		if (room)
			room->DestroyRoom();
		RoomGridFlatArray[i] = nullptr;
	}


	// Destroy eyes
	for (auto eyeTile : Gamemode->ActiveEyeTiles)
		eyeTile->OccupyingActor->Destroy();
	Gamemode->ActiveEyeTiles.Empty(6);
	Gamemode->EyeSpawns.Empty(6);

	// Reverse-iterate destruction list to avoid alloc errors
	for (int i = DestructionList.Num() - 1; i >= 0; --i)
		DestructionList[i]->Destroy();
	DestructionList.Empty();
}

void ADungeonMacroGrid::GenerateBossRoom()
{
	// Places corner rooms, the in between spots, then fills in the rest with the space.
	// Get the positions for the corners
	// Jump table for positions (bl, br, tl, tr)
	if (BossRoomList.Num() >= 9)
	{
		static const FVector2D corners[4] =
		{
			FVector2D(0, 0),
			FVector2D(0, MapMaxWidth - 1),
			FVector2D(MapMaxHeight - 1, 0),
			FVector2D(MapMaxHeight - 1, MapMaxWidth - 1)
		};
		// Add corner rooms
		AddRoom(corners[0], BossRoomList[0]);
		AddRoom(corners[1], BossRoomList[1]);
		AddRoom(corners[2], BossRoomList[2]);
		AddRoom(corners[3], BossRoomList[3]);
		// Fill each edge
		// North edge
		for (int y = 1; y < corners[3].Y; ++y) AddRoom(corners[2] + FVector2D(0, y), BossRoomList[4]);
		// South edge
		for (int y = 1; y < corners[1].Y; ++y) AddRoom(corners[0] + FVector2D(0, y), BossRoomList[5]);
		// East edge
		for (int x = 1; x < corners[3].X; ++x) AddRoom(corners[1] + FVector2D(x, 0), BossRoomList[6]);
		// West edge
		for (int x = 1; x < corners[3].X; ++x) AddRoom(corners[0] + FVector2D(x, 0), BossRoomList[7]);
		// Fill remaining space
		for (int x = 1; x < corners[3].X; ++x)
			for (int y = 1; y < corners[3].Y; ++y)
				AddRoom(FVector2D(x, y), BossRoomList[8]);
		// Building complete
	}
}
