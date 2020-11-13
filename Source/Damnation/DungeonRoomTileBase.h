// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DungeonSingleTile.h"
#include "DungeonTormentor.h"
#include "DungeonCrawlerEnemy.h"
#include "GameFramework/Actor.h"

// DEBUG PURPOSES
#if !UE_BUILD_SHIPPING
#include "Engine/Engine.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/KismetSystemLibrary.h"
#endif // !UE_BUILD_SHIPPING

#include "DungeonRoomTileBase.generated.h"

class ADungeonMacroGrid;

USTRUCT(BlueprintType)
struct FEnemySpawnData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D Spawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<ADungeonCrawlerEnemy> Type;
};

USTRUCT(BlueprintType)
struct FEnemySpawnDataArrayContainer
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FEnemySpawnData> DataArray;
};

UCLASS()
class DAMNATION_API ADungeonRoomTileBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADungeonRoomTileBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	const static int GridEdgeLength = 15;
	const static int RoomTileCount = GridEdgeLength * GridEdgeLength;
	const static int TileSeparation = 100;
	const static int RoomPositionScalar = GridEdgeLength * TileSeparation;

	// Helper to simplify cardinal setting
	inline static void CheckSetCardinal(int arrayPosA, int arrayPosB, ADungeonSingleTile* tileA, ADungeonSingleTile* tileB)
	{
		tileA->CardinalConnections[arrayPosA] = tileB;
		tileB->CardinalConnections[arrayPosB] = tileA;
	}

	// Forcefully unlinks a given tile from the direction. Should be avoided unless necessary.
	// Returns the tile that has been disconnected from the position, to allow storage to reconnect later.
	UFUNCTION(BlueprintCallable)
	ADungeonSingleTile* ForceTileDisconnect(ECardinal direction, ADungeonSingleTile* targetTile);

	// Forcefully links a given tile to the provided linkingTile. Should be avoided unless necessary.
	// this will overwrite the direction if one is already there, so use with care.
	UFUNCTION(BlueprintCallable)
	void ForceTileConnect(ECardinal direction, ADungeonSingleTile* targetTile, ADungeonSingleTile* linkingTile);

	// Event for post map texture load.
	UFUNCTION(BlueprintImplementableEvent, Category = "RoomTileBase")
	void OnMapLoad();

	// Called after all other functionality the room needs to be fully functional is called.
	UFUNCTION(BlueprintImplementableEvent, Category = "RoomTileBase")
	void OnMapFinalization();

	UFUNCTION(BlueprintCallable)
	void SpawnEnemy(FEnemySpawnData& data);

	// If this room has valid spawns & valid parameters, spawn starter enemies.
	UFUNCTION(BlueprintCallable)
	void SpawnEnemies(FEnemySpawnDataArrayContainer spawnInfo);

	UFUNCTION(BlueprintImplementableEvent, Category = "RoomTileBase")
	void OnEnemyDeath();

	UFUNCTION(BlueprintCallable)
	void SetMacroGrid(ADungeonMacroGrid* grid) { MacroGrid = grid; }

	UFUNCTION(BlueprintCallable)
	ADungeonSingleTile* AddTile(FVector2D position);

	UFUNCTION(BlueprintCallable)
	ADungeonTileOccupant* AddTileEntity(TSubclassOf<ADungeonTileOccupant> type, FVector2D position);

	UFUNCTION(BlueprintPure)
	static int GetGridEdgeLength() { return GridEdgeLength; }
	UFUNCTION(BlueprintPure)
	static int GetTileSeparation() { return TileSeparation; }
	UFUNCTION(BlueprintPure)
	static int GetRoomPositionScalar() { return RoomPositionScalar; }

	UFUNCTION(BlueprintPure)
	static FVector2D FlatToGridIndex(int flatIndex);
	UFUNCTION(BlueprintPure)
	static int GridToFlatIndex(FVector2D position);

	UFUNCTION(BlueprintPure)
	ADungeonSingleTile* GetTile(FVector2D position);

	// Gets a random valid tile from this room. Optionally supply a size requirement for the tile to fit a specific entity.
	UFUNCTION(BlueprintPure)
	ADungeonSingleTile* GetTileRandom(int size = 1);

	UFUNCTION(BlueprintPure)
	FVector GetRoomTilePosition(FVector2D position);

	// Get tile by flat array index
	UFUNCTION(BlueprintPure)
	ADungeonSingleTile* GetTileByIndex(int index);

	UFUNCTION(BlueprintCallable)
	void LoadTextureToMap();

	UFUNCTION(BlueprintCallable)
	void AssignSizes();

	UFUNCTION()
	void DestroyRoom();

	// The cardinal directions this room is able to connect to.
	// Only elements 0-3 will ever be read, thus the array size shouldn't be modified.
	// Each bool is evaluated clockwise (0 == North, 1 == East...)

	UPROPERTY(EditDefaultsOnly, EditFixedSize, Category = "Dungeon Room Data")
	TArray<bool> ValidCardinals;

	// The cardinal directions this room is currently connected to.
	//UPROPERTY(VisibleAnywhere)
	//FCardinalRoomTiles CardinalConnections;

	//UPROPERTY(VisibleAnywhere)
	//ADungeonMacroGrid* MacroGrid;

	// The texture that will be read for tile data.
	// Resolution must be equal to GridEdgeLength^2 in a square aspect ratio.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon Room Data")
	UTexture2D* MapTexture;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	int FlatArraySize = 0;

	UPROPERTY()
	FEnemySpawnDataArrayContainer SpawnDataContainer;

	// 2D array of tiles for movement
	UPROPERTY(BlueprintReadOnly)
	TArray<ADungeonSingleTile*> TileGridFlatArray;

	// The minimum amount of enemies that can be spawned in this room.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon Room Data")
	int EnemySpawnsMinimum = 0;

	// The maximum amount of enemies that can be spawned in this room. Must have enough spawn points to support this many enemies.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon Room Data")
	int EnemySpawnsMaximum = 2;

	// The minimum distance between the player & the spawn location before allowing the respawn to occur.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon Room Data")
	float RespawnMinDistance = 1000.0f;

	// The interval between respawn checks.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon Room Data")
	float RespawnInterval = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon Room Data")
	float RespawnAwaitingInterval = 10.0f;

	UPROPERTY(VisibleAnywhere, Category = "Dungeon Room Data")
	float CurrentRespawnInterval = 0.0f;

	// The number of enemies to spawn on the next respawn interval.
	UPROPERTY(BlueprintReadWrite)
	int RespawnCurrentCount;

	UPROPERTY(BlueprintReadOnly)
	ADungeonMacroGrid* MacroGrid;
};
