// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonHelpers.h"

ECardinal UDungeonHelpers::RotateCardinal(ECardinal start, int val)
{
	// Convert to int for maths
	int startI = (int)start;
	// Mod to ensure -4..4 range, then add 4 for range 0..8
	val = (val % 4) + 4;
	// Now range is 0..8, add val to startI & mod to get offset value
	return ECardinal((startI + val) % 4);
}

ECardinal UDungeonHelpers::ClosestDirection(const FVector2D& A, const FVector2D& B)
{
	float dirVec = (FMath::RadiansToDegrees(FMath::Atan2(B.Y - A.Y, B.X - A.X))) + 360.0f;
	return ECardinal((int((dirVec / 90.0f) + .5f)) % 4);
}
