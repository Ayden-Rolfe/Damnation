// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonEye.h"

ADungeonEye::ADungeonEye()
{
	Type = EOccupantType::Eye;

	PrimaryActorTick.bCanEverTick = false;
}

void ADungeonEye::Collect()
{
	//CurrentTile->OccupyingActor = nullptr;
	OnCollect();
	//Destroy();
}
