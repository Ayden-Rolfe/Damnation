// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DungeonTileOccupant.h"
#include "DungeonEye.generated.h"

/**
 * 
 */
UCLASS()
class DAMNATION_API ADungeonEye : public ADungeonTileOccupant
{
	GENERATED_BODY()

public:
	ADungeonEye();

	// Event for collection
	UFUNCTION(BlueprintImplementableEvent, Category = "Dungeon Eye")
	void OnCollect();


	UFUNCTION(BlueprintCallable)
	void Collect();

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* EyeMesh;
};
