// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"

// DEBUG PURPOSES
#if !UE_BUILD_SHIPPING
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/TextRenderComponent.h"
#endif // !UE_BUILD_SHIPPING

#include "DungeonSingleTile.generated.h"

UENUM(BlueprintType)
enum class ECardinal : uint8
{
	NORTH = 0 UMETA(DisplayName = "North"),
	EAST = 1 UMETA(DisplayName = "East"),
	SOUTH = 2 UMETA(DisplayName = "South"),
	WEST = 3 UMETA(DisplayName = "West"),
	CARDINALCOUNT = 4 UMETA(Hidden),
	NULLDIR = 0xFF UMETA(Hidden)
};

// Simple shuffle algorithm for a given TArray
template <class T>
static void ShuffleArray(TArray<T>& arr)
{
	if (arr.Num() > 0)
	{
		int32 lastIDX = arr.Num() - 1;
		for (int32 i = 0; i <= lastIDX; ++i)
		{
			int32 idx = FMath::RandRange(i, lastIDX);
			if (i != idx)
				arr.Swap(i, idx);
		}
	}
}

UCLASS()
class DAMNATION_API ADungeonSingleTile : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ADungeonSingleTile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Checks every tile adjacent to this, cardinally & diagonally
	// returns true if tiles exist, false otherwise
	UFUNCTION(BlueprintPure)
	bool CheckSurroundingTiles();

	// Get reference to connected tile in direction
	// Null if no tile is connected in that direction
	UFUNCTION(BlueprintPure)
	ADungeonSingleTile* GetConnectedTile(ECardinal Direction);

	/*
	* Returns an array of all valid tiles adjacent to this tile
	* including cardinally & diagonally, nullptr if no tile exists
	* Diagonals are automatically assigned nullptr if all cardinals are null
	* Array is arranged as such:
	*
	* [4][0][5]
	* [3][X][1]
	* [7][2][6]
	*/
	UFUNCTION(BlueprintPure)
	void GetSurroundingTiles(TArray<ADungeonSingleTile*>& outArray);

	// Get array of all 4 directions randomly shuffled.
	// Can be iterated through to check all tiles randomly in an efficient manner.
	UFUNCTION(BlueprintPure)
	static TArray<ECardinal> MakeRandDirectionArray();

	// Gets the adjacent tile in the given direction, or nullptr if no tile.
	UFUNCTION(BlueprintPure)
	ADungeonSingleTile* GetAdjacent(ECardinal direction);

	UPROPERTY(BlueprintReadOnly)
	TArray<ADungeonSingleTile*> CardinalConnections;

	//UPROPERTY(VisibleAnywhere)
	//UTextRenderComponent* DEBUG_IndexText;

	// The actor currently occupying this tile. Nullptr if none
	UPROPERTY(BlueprintReadWrite)
	AActor* OccupyingActor = nullptr;

	// UDELEGATE(BlueprintAuthorityOnly)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTileEventCall, AActor*, Occupant, ADungeonSingleTile*, TriggeredTile);

	UPROPERTY(BlueprintAssignable, Category="TileInfo")
	FTileEventCall TileEvent;

	// Tells pathfinding this tile is usable for any pathfinding
	// Also adjusts nearby tiles to account for this tiles' impassibility.
	UFUNCTION(BlueprintCallable)
	void PermitPathing(bool AllowPathing) 
	{
		TArray<ADungeonSingleTile*> tileList;
		GetSurroundingTiles(tileList);
		bPathingIgnore = !AllowPathing;
		if (AllowPathing)
		{
			// If pathing allowed, ensure nearby tiles are updated to match the new space available.
			for (auto tile : tileList)
				if (tile)
					tile->availableSpace = (tile->CheckSurroundingTiles()) ? 3 : 1;
			// And update this tile accordingly.
			availableSpace = CheckSurroundingTiles() ? 3 : 1;
		}
		else
		{
			// Pathing disallowed, nearby tiles can therefore only fit 1x1 occupants.
			for (auto tile : tileList)
				if (tile)
					tile->availableSpace = 1;
		}
	}

	bool bPathingIgnore = false;
	// Pathfinding variables/functions
	// Not actually used by this tile
	ADungeonSingleTile* parent;
	int availableSpace = -1;
	float gCost;
	float hCost;
	float fCost() const { return gCost + hCost; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};