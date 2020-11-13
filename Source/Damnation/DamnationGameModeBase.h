// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "DungeonMacroGrid.h"
#include "DungeonCrawlerPlayer.h"
#include "DamnationGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class DAMNATION_API ADamnationGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	ADungeonMacroGrid* DungeonMap = nullptr;

	// Randomly generates a dungeon map using tiles in the macro grid.
	UFUNCTION(BlueprintCallable)
	void InitDungeonMap(bool bSpawnPlayer = true);

	// Generate the boss room map grid.
	UFUNCTION(BlueprintCallable)
	void InitBossMap();

	UFUNCTION(BlueprintCallable)
	void DoEnemyMovement();

	// Checks the dead enemy list and cleans them up. Called automatically after each player turn & by the tormentor after an attack.
	UFUNCTION(BlueprintCallable)
	void CleanupDeadEnemies();

	UFUNCTION(BlueprintCallable)
	void DoTormentorAction();

	UFUNCTION(BlueprintCallable)
	TArray<ADungeonSingleTile*> CallPathfinder(ADungeonSingleTile* start, ADungeonSingleTile* end, int size = 1, bool GoForClosest = true, bool respectOccupants = false);

	UFUNCTION(BlueprintCallable)
	void SetPlayerLocation(ADungeonSingleTile* target);

	UFUNCTION(BlueprintCallable)
	void AddTormentorLocation(ADungeonSingleTile* tile);

	UFUNCTION(BlueprintCallable)
	void AddEyeLocation(ADungeonRoomTileBase* room, FVector2D position);

	UFUNCTION(BlueprintCallable)
	void DespawnEnemies();

	UFUNCTION(BlueprintPure)
	ADungeonCrawlerPlayer* GetPlayer() { return ActivePlayer; }

	UFUNCTION(BlueprintPure)
	ADungeonTormentor* GetTormentor() { return ActiveTormentor; }

	// Action time is set by player object, so this is obtained from them
	// If they don't exist, returns a filler value of 0.1
	UFUNCTION(BlueprintPure)
	float GetActionTime() { return ActivePlayer ? ActivePlayer->GetActionTime() : 0.1f; }

	UFUNCTION(BlueprintNativeEvent)
	void GenerateMinimap();

	// Place eyes in the map
	void PlaceEyes(ADungeonRoomTileBase* ReqRoom = nullptr);

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class ADungeonSingleTile> TileType;

	// The number of eyes the game will attempt to spawn.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int EyeCount = 6;

	// The minimum distance between each eye.
	UPROPERTY(EditDefaultsOnly)
	float EyeSpacingDistance = 1000.0f;

	// The minimum distance from the player for the enemy despawn function to despawn an enemy.
	UPROPERTY(EditDefaultsOnly)
	float DespawnDistance = 1500.0f;

	// The type to spawn on green tiles
	UPROPERTY(EditDefaultsOnly, Category = "Dungeon Variables|Required Class Types")
	TSubclassOf<ADungeonCrawlerPlayer> PlayerActorType;

	// The type to spawn on red tiles
	UPROPERTY(EditDefaultsOnly, Category = "Dungeon Variables|Required Class Types")
	TSubclassOf<ADungeonTormentor> TormentorActorType;

	UPROPERTY(EditDefaultsOnly, Category = "Dungeon Variables|Required Class Types")
	TSubclassOf<ADungeonEye> EyeActorType;

	// Called right before assignment of map to macro grid.
	// Use to add new delegate calls to colors.
	UFUNCTION(BlueprintImplementableEvent)
	void BindColorMapEvents();

	// Called when the player begins the game.
	UFUNCTION(BlueprintImplementableEvent)
	void BeginGame();

	// Called when the player performs some kind of action.
	UFUNCTION(BlueprintImplementableEvent)
	void PlayerAction();

	DECLARE_DYNAMIC_DELEGATE_TwoParams(FTileColorSpawn, ADungeonRoomTileBase*, room, FVector2D, position);

	UFUNCTION(BlueprintCallable)
	void AddColorMap(FColor color, FTileColorSpawn callback)
	{
		ColorDelegateMap.Add(color, callback);
	}

	// Make color with forced 255 alpha to ensure bitmap compatibility.
	UFUNCTION(BlueprintPure)
	static FColor MakeRGBColor(uint8 InR, uint8 InG, uint8 InB)
	{
		return FColor(InR, InG, InB);
	}

	TMap<FColor, FTileColorSpawn> ColorDelegateMap;

	UPROPERTY(BlueprintReadOnly)
	ADungeonCrawlerPlayer* ActivePlayer;
	UPROPERTY(BlueprintReadOnly)
	ADungeonSingleTile* LastTile = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<ADungeonCrawlerEnemy*> ActiveEnemies;
	UPROPERTY(BlueprintReadOnly)
	TArray<ADungeonCrawlerEnemy*> ToBeKilledEnemies;

	// Array of active projectiles.
	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> ActiveProjectiles;

	UPROPERTY(BlueprintReadWrite)
	ADungeonTormentor* ActiveTormentor;
	UPROPERTY(BlueprintReadOnly)
	TArray<ADungeonSingleTile*> TormentorSpawnLocations;

	TArray<TPair<FVector2D, ADungeonRoomTileBase*>> EyeSpawns;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ADungeonSingleTile*> ActiveEyeTiles;
};
