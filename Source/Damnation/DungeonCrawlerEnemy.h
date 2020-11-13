// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DungeonTileOccupant.h"
#include "DungeonCrawlerEnemy.generated.h"

class ADungeonCrawlerPlayer;

UCLASS()
class DAMNATION_API ADungeonCrawlerEnemy : public ADungeonTileOccupant
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADungeonCrawlerEnemy();

	UFUNCTION()
	void PerformMovement();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Move Action"))
	void ReceiveMoveAction();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Take Damage"))
	void ReceiveTakeDamage();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Death"))
	void ReceiveOnDeath();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Despawn"))
	void ReceiveOnDespawn();

	// Force repath
	UFUNCTION(BlueprintCallable)
	void SetTarget(ADungeonSingleTile* Target);

	// Sets facing direction of model root
	UFUNCTION(BlueprintCallable)
	void SetModelFacing(ECardinal direction);

	// Alert enemy to the target moving 1 tile, but don't force repath
	// Will decide whether repath is necessary or not
	UFUNCTION(BlueprintCallable)
	void AlertTarget(ADungeonSingleTile* Target);

	UFUNCTION(BlueprintCallable)
	void AlterHealth(int amount);

	UFUNCTION(BlueprintCallable)
	void ClearTileAndSelf();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite)
	class ADungeonRoomTileBase* OriginRoom;

protected:
	// The time between forced repath executions
	UPROPERTY(EditDefaultsOnly)
	int MaxRepathInterval;

	// The remaining intervals until a forced repath
	UPROPERTY(BlueprintReadOnly)
	int RepathInterval;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int Health = 1;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// The path to the current target, with [0] being the next desired tile.
	UPROPERTY(BlueprintReadOnly)
	TArray<ADungeonSingleTile*> DesiredPath;
};
