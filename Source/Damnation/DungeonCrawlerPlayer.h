// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DungeonCrawlerEnemy.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DungeonCrawlerPlayer.generated.h"

class ADungeonSingleTile;

UENUM(BlueprintType)
enum class EPlayerAction : uint8
{
	NOACTION = 0 UMETA(DisplayName = "No Action"),
	ACTION1 = 1 UMETA(DisplayName = "Action 1"),
	ACTION2 = 2 UMETA(DisplayName = "Action 2"),
	ACTION3 = 3 UMETA(DisplayName = "Action 3"),
	ACTIONCOUNT = 4 UMETA(Hidden)
};

UCLASS()
class DAMNATION_API ADungeonCrawlerPlayer : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADungeonCrawlerPlayer();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void SetGamemode(ADamnationGameModeBase* gm) { Gamemode = gm; }

	UFUNCTION(BlueprintPure)
	float GetActionTime() { return ActionTime; }

	// Directly set the players' HP
	UFUNCTION(BlueprintCallable)
	void AlterHealth(int val);
	UFUNCTION(BlueprintPure)
	int GetHealth() { return Health; }
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Health Altered"))
		void ReceiveOnHealthAltered(bool positiveGain);

	UFUNCTION(BlueprintCallable)
	void AddEye() { EyeCount++; }
	UFUNCTION(BlueprintPure)
	int GetEyeCount() { return EyeCount; }

	// Can the player move right now?
	UFUNCTION(BlueprintPure)
	inline bool CanMove() { return TimeUntilActionPermitted <= 0.0f && !bLockInput; }

	// Called when player does a movement action.
	// Positions refer to the player pre- and post- movement action, respectively.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Move"))
	void ReceiveOnMove(FVector oldPos, FVector newPos, ECardinal newDir);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Action"))
	void ReceiveOnAction(ECardinal direction, EPlayerAction action, ADungeonSingleTile* target);

	// Called after any perform, including both action & movement.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Perform"))
	void ReceiveOnPerform();

	// Called when the player changes their action input.
	// Action is equal to the most recent valid action the player wants to perform.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Action Update"))
	void ReceiveOnActionUpdate(EPlayerAction action);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Death"))
	void ReceiveOnDeath();

	// The current tile the player is standing on
	UPROPERTY(BlueprintReadWrite)
	ADungeonSingleTile* CurrentTile;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Movement function
	void DoAction(ECardinal cardinal);

	// Buffer modification functions called by inputs
	void SetCurrentAction(EPlayerAction action);
	void CancelAction(EPlayerAction action);
	void SetCurrentMovement(ECardinal direction);
	void CancelMovement(ECardinal direction);

	// Empties the action & movement buffer, ensuring only the null indicators are left.
	UFUNCTION(BlueprintCallable)
		void EmptyBuffers();
	// Get the current top-level player action.
	UFUNCTION(BlueprintPure)
		EPlayerAction GetCurrentAction();

	// Create delegate with parameter to pass bindaction with int into DoAction() & SetBool()
	DECLARE_DELEGATE_OneParam(FMovementSetDelegate, ECardinal);
	DECLARE_DELEGATE_OneParam(FActionSetDelegate, EPlayerAction);

	UFUNCTION(BlueprintCallable)
	void JumpToTile(ADungeonSingleTile* nextTile, ECardinal moveDir);

	// Buffers for player actions/movement
	TArray<EPlayerAction, TFixedAllocator<(uint8)EPlayerAction::ACTIONCOUNT>> ActionBuffer;
	TArray<ECardinal, TFixedAllocator<(uint8)ECardinal::CARDINALCOUNT>> MovementBuffer;

	// UPROPERTY(VisibleAnywhere)
	// EPlayerAction currentAction;

	UPROPERTY(BlueprintReadWrite)
	USceneComponent* LaggedRoot;

	// Players' max health
	UPROPERTY(EditDefaultsOnly)
	int MaxHealth = 1000;

	// The time it takes to do a single action.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ActionTime = 0.5f;

	// The current delay before the next action can be taken.
	// <= 0.0 means an action is permitted.
	UPROPERTY(VisibleAnywhere)
	float TimeUntilActionPermitted = 0.0f;

	// A scalar version of TimeUntilActionPermitted
	// When action is performed, this is set to 1 and descends to 0 over the time from ActionTime.
	UPROPERTY(BlueprintReadOnly)
	float ActionTimeScalar = 0.0f;

	UPROPERTY(VisibleAnywhere)
	FVector OldPosition;

	// The players' health.
	UPROPERTY(VisibleAnywhere)
	int Health;

	// The number of eyes the player has collected so far.
	UPROPERTY(VisibleAnywhere)
	int EyeCount = 0;

	// If Enabled, the player is prevented from acting.
	UPROPERTY(BlueprintReadWrite)
	bool bLockInput;

	// An array of actions being buffered
	// toggles each switch based on input
	TArray<bool, TFixedAllocator<4>> bufferActionDir;

	ADamnationGameModeBase* Gamemode;
};
