// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonSingleTile.h"
#include "Kismet/GameplayStatics.h"
#include "DungeonTileOccupant.generated.h"

class ADamnationGameModeBase;

UENUM(BlueprintType)
enum EOccupantType
{
	Boss,
	Enemy,
	Eye
};

UCLASS(abstract)
class DAMNATION_API ADungeonTileOccupant : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADungeonTileOccupant();

	// Called every frame
	virtual void Tick(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	EOccupantType GetType() { return Type; }

	UFUNCTION(BlueprintCallable)
	virtual void SetTile(ADungeonSingleTile* tile);

	// Does a raycast from this to the player, returns hit result that will hit any SightBlocker blocking collider
	UFUNCTION(BlueprintCallable)
	FHitResult SightCheck();

	// Called when a movement action is performed
	// Positions refer to the player pre- and post- movement action, respectively
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnMove"))
	void ReceiveOnMove(FTransform oldPos, FTransform newPos);

	// The current tile this is placed on
	UPROPERTY(BlueprintReadWrite)
	ADungeonSingleTile* CurrentTile;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override { Super::BeginPlay(); }

	UPROPERTY(BlueprintReadWrite)
	USceneComponent* LaggedRoot;

	// The time it takes to do a single action. Set by Gamemode on spawn.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Occupant Variables")
	float ActionTime = 0.25f;

	// The current delay before the next action can be taken.
	// <= 0.0 means an action is permitted.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Occupant Variables")
	float TimeUntilActionPermitted = 0.0f;

	// A scalar version of TimeUntilActionPermitted
	// When action is performed, this is set to 1 and descends to 0 over the time from ActionTime.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Occupant Variables")
	float ActionTimeScalar = 0.0f;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Occupant Variables")
	FTransform OldTransform;

	EOccupantType Type;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	ADamnationGameModeBase* Gamemode;
};
