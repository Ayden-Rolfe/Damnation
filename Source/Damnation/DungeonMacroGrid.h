// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetArrayLibrary.h"
#include "DungeonRoomTileBase.h"
#include "DungeonEye.h"
#include "DungeonMacroGrid.generated.h"

UCLASS()
class DAMNATION_API ADungeonMacroGrid : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADungeonMacroGrid();

	UFUNCTION(BlueprintCallable)
	void SetGamemode(ADamnationGameModeBase* gm) { Gamemode = gm; }
	UFUNCTION(BlueprintPure)
	ADamnationGameModeBase* GetGamemode() { return Gamemode; }

	// Propagation functions to set player location/add tormentor spawn
	UFUNCTION(BlueprintCallable)
	void SetPlayerLocation(ADungeonSingleTile* tile);
	UFUNCTION(BlueprintCallable)
	void AddTormentorLocation(ADungeonSingleTile* tile);

	// Convert a grid position to world position, returns center of possible room
	UFUNCTION(BlueprintPure)
	static FVector ConvertPosition(FVector2D position);

	UFUNCTION(BlueprintCallable)
	ADungeonRoomTileBase* AddRoom(FVector2D position, TSubclassOf<ADungeonRoomTileBase> roomType);

	UFUNCTION(BlueprintPure)
	ADungeonRoomTileBase* GetRoom(FVector2D position);

	UFUNCTION(BlueprintPure)
	ADungeonRoomTileBase* GetRoomByIndex(int index);

	UFUNCTION(BlueprintPure)
	ADungeonRoomTileBase* GetRoomByWorldPosition(FVector position);

	UFUNCTION(BlueprintPure)
	ADungeonRoomTileBase* GetRoomRandom();

	UFUNCTION(BlueprintPure)
	bool IsValidSpace(FVector2D position);

	UFUNCTION(BlueprintCallable)
	TArray<ADungeonSingleTile*> GeneratePath(ADungeonSingleTile* start, ADungeonSingleTile* end, int actorSize = 1, bool getClosest = true, bool respectOccupants = false);

	UFUNCTION(BlueprintPure)
	FVector2D FlatToGridIndex(int index);
	UFUNCTION(BlueprintPure)
	int GridToFlatIndex(FVector2D position);
	// Attempts to convert a world position to a room position.
	UFUNCTION(BlueprintPure)
	static FVector2D WorldToRoomPosition(FVector position);
	// Attempts to convert a world position to a tile position.
	UFUNCTION(BlueprintPure)
	static FVector2D WorldToTilePosition(FVector position);
	// Gets the tile at the specified world position. May be null.
	UFUNCTION(BlueprintPure)
	ADungeonSingleTile* GetTileAtWorldPosition(FVector position);

	// Generates dungeon floor map
	void GenerateFloor();

	// Completely wipes the dungeon and all related objects. Designed specifically for demo purposes.
	UFUNCTION(BlueprintCallable)
	void DestroyGeneration();

	// Generates the boss room
	void GenerateBossRoom();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Generation")
	int MapMaxWidth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Generation")
	int MapMaxHeight;

	// The position of the starting room on the grid.
	// Ideally placed somewhere below the Escape Room Position for best path results.
	UPROPERTY(EditAnywhere, Category = "Map Generation")
	FVector2D StarterRoomPosition = FVector2D(0, 7);

	// The position of the escape room on the grid.
	// Must be placed in a location where the spot below is a usable tile.
	UPROPERTY(EditAnywhere, Category = "Map Generation")
	FVector2D EscapeRoomPosition = FVector2D(10, 7);

	// The minimum chance for a random-chance connector to be valid
	UPROPERTY(EditAnywhere, Category = "Map Generation|Fill Values")
	float minFillChance = 0.1f;
	// The starting chance for a random-chance connector to be valid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generation|Fill Values")
	float maxFillChance = 1.0f;
	// The amount which is subtracted from the current fill chance upon a successful connection being established by chance.
	UPROPERTY(EditAnywhere, Category = "Map Generation|Fill Values")
	float fillChanceVelocity = 0.1f;
	// The biases for each connection direction, relative to the direction of the connector currently being extended from.
	// Indexes 0-3 of the array == relative direction, value is multiplied by connector chance.
	// NOTE: index 2 is ignored as it is guaranteed to connect regardless of settings.
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "Map Generation|Fill Values")
	TArray<float> DirectionalBiases;

	// The map tile used for the starting room.
	UPROPERTY(EditAnywhere, Category = "Map Generation|Classes")
	TSubclassOf<ADungeonRoomTileBase> StarterRoom;
	// The map tile used immediately after the starting room.
	UPROPERTY(EditAnywhere, Category = "Map Generation|Classes")
	TSubclassOf<ADungeonRoomTileBase> TutorialRoom;
	// The map tile placed one below the top middle.
	UPROPERTY(EditAnywhere, Category = "Map Generation|Classes")
	TSubclassOf<ADungeonRoomTileBase> MuralRoom;
	// The map tile placed in the top middle.
	UPROPERTY(EditAnywhere, Category = "Map Generation|Classes")
	TSubclassOf<ADungeonRoomTileBase> EscapeRoom;
	// The list of rooms the generation system can utilise. 
	UPROPERTY(EditAnywhere, Category = "Map Generation|Classes")
	TArray<TSubclassOf<ADungeonRoomTileBase>> RoomList;

	// The rooms used to generate the boss room.
	// MUST USE THE FOLLOWING FORMAT OF ROOMS:
	// 0..3: corner rooms in order bl, br, tl, tr
	// 4..7: edge rooms with walls in order North, South, East, West
	// 8: Filler room with all sides available
	UPROPERTY(EditAnywhere, Category = "Map Generation|Classes")
	TArray<TSubclassOf<ADungeonRoomTileBase>> BossRoomList;

	// List of actors to destroy when instructed. For demo purposes.
	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> DestructionList;

	// Get fill chance & modify by velocity
	float GetFill();

	// Map of colours on map to spawns
	// TMap<FColor, FTileColorSpawn> ColorItemMap;

	// Map of delegates TEST
	// TMap<FColor, FTileColorSpawn> ColorDelegateMap;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	ADamnationGameModeBase* Gamemode;

	// 2D array of rooms
	UPROPERTY()
	TArray<ADungeonRoomTileBase*> RoomGridFlatArray;

	int ArrayWidth = 0;
	int ArrayHeight = 0;
	int FlatArraySize = 0;

	inline static void ConnectNorthRooms(ADungeonRoomTileBase* roomA, ADungeonRoomTileBase* roomB);
	inline static void ConnectEastRooms(ADungeonRoomTileBase* roomA, ADungeonRoomTileBase* roomB);
	inline static void ConnectSouthRooms(ADungeonRoomTileBase* roomA, ADungeonRoomTileBase* roomB);
	inline static void ConnectWestRooms(ADungeonRoomTileBase* roomA, ADungeonRoomTileBase* roomB);

	UPROPERTY()
	USceneComponent* MapOrigin;
};
