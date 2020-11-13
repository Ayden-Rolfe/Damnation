// Fill out your copyright notice in the Description page of Project Settings.

#include "DungeonCrawlerPlayer.h"
#include "DamnationGameModeBase.h"
#include "DungeonSingleTile.h"

// Sets default values
ADungeonCrawlerPlayer::ADungeonCrawlerPlayer()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Accept input from player
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoReceiveInput = EAutoReceiveInput::Player0;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	LaggedRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Lagged Root"));
	LaggedRoot->SetupAttachment(RootComponent);

	Health = MaxHealth;

	// Init buffers
	MovementBuffer.Add(ECardinal::NULLDIR);
	ActionBuffer.Add(EPlayerAction::NOACTION);
}

// Called when the game starts or when spawned
void ADungeonCrawlerPlayer::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
}

void ADungeonCrawlerPlayer::DoAction(ECardinal cardinal)
{
	if (!CanMove())
		return;
	// enum to int for array access purposes
	uint8 iCardinal = (uint8)cardinal;
	// Ensure we're on a tile & that tile has another accessible tile in the direction specified
	if (!(CurrentTile && CurrentTile->CardinalConnections[iCardinal]))
		return;

	// Update oldpos to match current pos so lagged root doesn't wig out
	// Will be properly set later if necessary
	OldPosition = GetActorLocation();

	// Get the next tile
	ADungeonSingleTile* nextTile = CurrentTile->CardinalConnections[iCardinal];

	// Get latest action in buffer
	EPlayerAction nextAction = ActionBuffer.Last();
	// Check if player is attempting an action, or if the tile they're trying to go to is occupied
	if (nextAction == EPlayerAction::NOACTION && !nextTile->OccupyingActor)
	{
		JumpToTile(nextTile, cardinal);
	}
	else
	{
		// This must be an action, so if action buffer is empty use action 1
		nextAction = nextAction == EPlayerAction::NOACTION ? EPlayerAction::ACTION1 : nextAction;
		ReceiveOnAction(cardinal, nextAction, nextTile);
	}

	// Alert Gamemode that the player has moved, and that enemies should do the same
	if (Gamemode)
		Gamemode->DoEnemyMovement();
	TimeUntilActionPermitted = ActionTime;
	ReceiveOnPerform();
}


// Buffer modification functions called by inputs

void ADungeonCrawlerPlayer::SetCurrentAction(EPlayerAction action)
{
	ActionBuffer.Add(action);
	ReceiveOnActionUpdate(action);
}

void ADungeonCrawlerPlayer::CancelAction(EPlayerAction action)
{
	ActionBuffer.Remove(action);
	ReceiveOnActionUpdate(ActionBuffer.Last());
}

void ADungeonCrawlerPlayer::SetCurrentMovement(ECardinal direction)
{
	MovementBuffer.Add(direction);
}

void ADungeonCrawlerPlayer::CancelMovement(ECardinal direction)
{
	MovementBuffer.Remove(direction);
}

void ADungeonCrawlerPlayer::EmptyBuffers()
{
	MovementBuffer.RemoveAll([](ECardinal dir) {return dir != ECardinal::NULLDIR; });
	ActionBuffer.RemoveAll([](EPlayerAction action) {return action != EPlayerAction::NOACTION; });
}

EPlayerAction ADungeonCrawlerPlayer::GetCurrentAction()
{
	return ActionBuffer.Last();
}

void ADungeonCrawlerPlayer::JumpToTile(ADungeonSingleTile* nextTile, ECardinal moveDir)
{
	// Move player & assign necessary variables
	// OldTransform = LaggedRoot->GetComponentLocation();
	OldPosition = GetActorLocation();
	SetActorLocation(nextTile->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
	LaggedRoot->SetWorldLocation(OldPosition);
	CurrentTile->OccupyingActor = nullptr;
	nextTile->OccupyingActor = this;
	CurrentTile = nextTile;
	ReceiveOnMove(OldPosition, GetActorLocation(), moveDir);
	nextTile->TileEvent.Broadcast(this, nextTile);
}

// Called every frame
void ADungeonCrawlerPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementBuffer.Num() == 0)
		MovementBuffer.Add(ECardinal::NULLDIR);
	if (MovementBuffer.Last() != ECardinal::NULLDIR)
		DoAction(MovementBuffer.Last());

	TimeUntilActionPermitted = FMath::Max(TimeUntilActionPermitted - DeltaTime, 0.0f);
	ActionTimeScalar = TimeUntilActionPermitted / ActionTime;

	FVector lerpPos = FMath::Lerp(GetActorLocation(), OldPosition, ActionTimeScalar);
	LaggedRoot->SetWorldLocation(lerpPos);
}



void ADungeonCrawlerPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction<FMovementSetDelegate>("MoveUp", EInputEvent::IE_Pressed, this, &ADungeonCrawlerPlayer::SetCurrentMovement, ECardinal::NORTH);
	PlayerInputComponent->BindAction<FMovementSetDelegate>("MoveUp", EInputEvent::IE_Released, this, &ADungeonCrawlerPlayer::CancelMovement, ECardinal::NORTH);
	PlayerInputComponent->BindAction<FMovementSetDelegate>("MoveRight", EInputEvent::IE_Pressed, this, &ADungeonCrawlerPlayer::SetCurrentMovement, ECardinal::EAST);
	PlayerInputComponent->BindAction<FMovementSetDelegate>("MoveRight", EInputEvent::IE_Released, this, &ADungeonCrawlerPlayer::CancelMovement, ECardinal::EAST);
	PlayerInputComponent->BindAction<FMovementSetDelegate>("MoveDown", EInputEvent::IE_Pressed, this, &ADungeonCrawlerPlayer::SetCurrentMovement, ECardinal::SOUTH);
	PlayerInputComponent->BindAction<FMovementSetDelegate>("MoveDown", EInputEvent::IE_Released, this, &ADungeonCrawlerPlayer::CancelMovement, ECardinal::SOUTH);
	PlayerInputComponent->BindAction<FMovementSetDelegate>("MoveLeft", EInputEvent::IE_Pressed, this, &ADungeonCrawlerPlayer::SetCurrentMovement, ECardinal::WEST);
	PlayerInputComponent->BindAction<FMovementSetDelegate>("MoveLeft", EInputEvent::IE_Released, this, &ADungeonCrawlerPlayer::CancelMovement, ECardinal::WEST);
	PlayerInputComponent->BindAction<FActionSetDelegate>("Action1", EInputEvent::IE_Pressed, this, &ADungeonCrawlerPlayer::SetCurrentAction, EPlayerAction::ACTION1);
	PlayerInputComponent->BindAction<FActionSetDelegate>("Action1", EInputEvent::IE_Released, this, &ADungeonCrawlerPlayer::CancelAction, EPlayerAction::ACTION1);
	PlayerInputComponent->BindAction<FActionSetDelegate>("Action2", EInputEvent::IE_Pressed, this, &ADungeonCrawlerPlayer::SetCurrentAction, EPlayerAction::ACTION2);
	PlayerInputComponent->BindAction<FActionSetDelegate>("Action2", EInputEvent::IE_Released, this, &ADungeonCrawlerPlayer::CancelAction, EPlayerAction::ACTION2);
	// Swipe attack no longer in use
	//PlayerInputComponent->BindAction<FActionSetDelegate>("Action3", EInputEvent::IE_Pressed, this, &ADungeonCrawlerPlayer::SetCurrentAction, EPlayerAction::ACTION3);
	//PlayerInputComponent->BindAction<FActionSetDelegate>("Action3", EInputEvent::IE_Released, this, &ADungeonCrawlerPlayer::CancelAction, EPlayerAction::ACTION3);
}

void ADungeonCrawlerPlayer::AlterHealth(int val)
{
	Health = FMath::Min(Health += val, MaxHealth);
	ReceiveOnHealthAltered((val > 0));
	if (Health <= 0)
	{
		// Death functionality here
		CurrentTile->OccupyingActor = nullptr;
		ReceiveOnDeath();
	}
}


