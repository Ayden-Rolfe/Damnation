// Fill out your copyright notice in the Description page of Project Settings.

#include "DungeonTileOccupant.h"
#include "DamnationGameModeBase.h"

// Sets default values
ADungeonTileOccupant::ADungeonTileOccupant()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Gamemode = dynamic_cast<ADamnationGameModeBase*>(UGameplayStatics::GetGameMode(GetWorld()));
	if (Gamemode)
		ActionTime = Gamemode->GetActionTime();

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	LaggedRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Lagged Root"));
	LaggedRoot->SetupAttachment(RootComponent);
}

void ADungeonTileOccupant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimeUntilActionPermitted = FMath::Max(TimeUntilActionPermitted - DeltaTime, 0.0f);
	ActionTimeScalar = TimeUntilActionPermitted / ActionTime;

	FTransform lerpPos = UKismetMathLibrary::TLerp(GetActorTransform(), OldTransform, ActionTimeScalar);
	LaggedRoot->SetWorldTransform(lerpPos);
}


FHitResult ADungeonTileOccupant::SightCheck()
{
	const TArray<AActor*> ignoreActors;
	FHitResult outHit;
	bool hitResult = UKismetSystemLibrary::LineTraceSingle
	(
		this,
		GetActorLocation() + FVector(0, 0, 100),
		Gamemode->GetPlayer()->GetActorLocation() + FVector(0, 0, 100),
		// Search sightline blocker channel
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel2),
		false,
		ignoreActors,
		EDrawDebugTrace::None,
		outHit,
		true
	);
	return outHit;
}

void ADungeonTileOccupant::SetTile(ADungeonSingleTile* tile)
{
	SetActorLocation(tile->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
	LaggedRoot->SetWorldTransform(OldTransform);
	if (CurrentTile) CurrentTile->OccupyingActor = nullptr;
	tile->OccupyingActor = this;
	CurrentTile = tile;
	ReceiveOnMove(OldTransform, GetActorTransform());
	tile->TileEvent.Broadcast(this, tile);
}
