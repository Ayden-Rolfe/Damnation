// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DungeonSingleTile.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DungeonHelpers.generated.h"

/**
 * 
 */
UCLASS()
class DAMNATION_API UDungeonHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// Rotates the supplied cardinal by val, which may be negative. Guaranteed to be safe.
	// e.g. North + 2 == South, East - 3 == South
	UFUNCTION(BlueprintPure)
	static ECardinal RotateCardinal(ECardinal start, int val);

	// Gets the closest matching cardinal direction from the directional vector of A - B.
	UFUNCTION(BlueprintPure)
	static ECardinal ClosestDirection(const FVector2D& A, const FVector2D& B);
};
