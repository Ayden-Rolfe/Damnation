// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DungeonTileOccupant.h"
#include "DungeonHelpers.h"

//DEBUG
#include "Kismet/KismetSystemLibrary.h"

#include "DungeonTormentor.generated.h"

UENUM(BlueprintType)
enum class ETormentorAttackType : uint8
{
	SWIPE = 0 UMETA(DisplayName = "Swipe Attack"),
	SLAM = 1 UMETA(DisplayName = "Slam Attack"),

	// NULLATK = 0xFF UMETA(Hidden)
};

// Called when tormentor dies, only applies during boss battle.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTormentorDeath);

// Triggered when the tormentor reaches the end of their path.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPathEnded);

// Triggered when the tormentor does movement.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoving);

// Triggered when the tormentor does an attack.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttack, ETormentorAttackType, AttackType);

// Triggered when the tormentor rotates. Direction is value of -1 or 1, depending on left or right turn, respectively.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRotate, int, Direction);

UCLASS()
class DAMNATION_API ADungeonTormentor : public ADungeonTileOccupant
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADungeonTormentor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void PerformMovement();

	UFUNCTION(BlueprintCallable)
	void RotateToDirection(const ECardinal& dir);

	// Alter the tormentors' health. Returns true if the attack was acknowledged (bIsVulnerable)
	UFUNCTION(BlueprintCallable)
	bool AlterHealth(int val);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Action"))
	void ReceiveMoveAction();

	// Called on the tormentors' turn if an bIsAttacking is true, overriding the normal movement logic.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Attack"))
	void ReceiveOnAttack(bool bInSwipeRange, bool bInSlamRange);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Rotate"))
	void ReceiveOnRotate(ECardinal OldRotation, ECardinal NewRotation);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On First Time Spotting Player"))
	void ReceiveOnSpotPlayer();

	// Activates when the tormentor is killed. Typically spells victory.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Death"))
	void ReceiveOnDeath();

	// Activates when the tormentor is hurt. Only triggered if they are vulnerable.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Hurt"))
	void ReceiveOnHurt();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Can't Reach Target"))
	void ReceiveOnCannotReachTarget();

	// Called every movement action. If the tormentor can see the player, CanSeePlayer is true, otherwise false.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Check Can See Player"))
	void ReceivePlayerSightCheck(bool CanSeePlayer);

	UPROPERTY(BlueprintAssignable)
	FOnTormentorDeath OnTormentorDeath;

	UPROPERTY(BlueprintAssignable)
	FOnPathEnded OnTormentorPathEndReached;

	UPROPERTY(BlueprintAssignable)
	FOnMoving OnTormentorMovementAction;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnAttack OnTormentorAttackAction;

	UPROPERTY(BlueprintAssignable)
	FOnRotate OnTormentorRotate;

	// Called when the tormentor attempts a swipe attack.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AttackSwipe();

	// Called when the tormentor attempts a slam attack.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AttackSlam();

	UFUNCTION(BlueprintCallable)
	bool SetTarget(ADungeonSingleTile* Target, bool GoForClosest = true);

	// UFUNCTION(BlueprintCallable)
	virtual void SetTile(ADungeonSingleTile* tile) override;

	UFUNCTION(BlueprintCallable)
	void SetTormentorMoveSpeed(float speed) { MoveDuration = speed; }
	UFUNCTION(BlueprintCallable)
	void SetTormentorRotateSpeed(float speed) { RotateDuration = speed; }

	// Duration of action when first spotting the player.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tormentor Variables|Movement")
	float SpotPlayerActionDuration = 1.0f;

	// Time taken for the tormentor to move 1 tile.
	// Measured in time taken to perform an action.
	// (1 / TMS) == actions per second.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tormentor Variables|Movement")
	float MoveDuration = 0.5f;
	
	// Time taken to turn 90 degrees in place.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tormentor Variables|Movement")
	float RotateDuration = 0.5f;

	// The maximum distance the tormentor can spot the player from.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tormentor Variables|Movement")
	float SpotDistanceMax = 2000.0f;

	// Time taken for the slam attack wind up.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tormentor Variables|Attacking")
	float SlamAttackWindupDuration = 0.5f;

	// Time taken for the slam attack to occur.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tormentor Variables|Attacking")
	float SlamAttackDuration = 0.5f;

	// Time taken for the swipe attack wind up.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tormentor Variables|Attacking")
	float SwipeAttackWindupDuration = 0.5f;

	// Time taken for the swipe attack to occur.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tormentor Variables|Attacking")
	float SwipeAttackDuration = 0.5f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool GetTilesDirectional(int direction, TArray<ADungeonSingleTile*>& tileList);

	bool GetSwipeAttackTiles(int direction, TArray<ADungeonSingleTile*>& tileList);

	UFUNCTION(BlueprintPure)
	bool GetSwipeAttackTiles(ECardinal direction, TArray<ADungeonSingleTile*>& tileList) { return GetSwipeAttackTiles((int)direction, tileList); }

	// Gets the tiles for a slam attack. Returns if the attack is valid to perform or not.
	bool GetSlamAttackTiles(int direction, TArray<ADungeonSingleTile*>& tileList);

	// Gets the tiles for a slam attack. Returns if the attack is valid to perform or not.
	UFUNCTION(BlueprintPure)
	bool GetSlamAttackTiles(ECardinal direction, TArray<ADungeonSingleTile*>& tileList) { return GetSlamAttackTiles((int)direction, tileList); }

	// Is the tormentor vulnerable to attacks?
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tormentor Variables")
	bool bIsVulnerable = false;

	// True on movement actions where the tormentor spotted the player.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Tormentor Variables")
	bool bCanSeePlayer = false;

	// Does the tormentor have an attack prepared?
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tormentor Variables")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tormentor Variables")
	bool bIsAttackPrepared = false;

	// True until the tormentor has seen the player for the first time.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tormentor Variables")
	bool bFirstPlayerSight = true;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Tormentor Variables")
	int MaxHealth;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Tormentor Variables")
	int Health = MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tormentor Variables|Attacking")
	bool bIsDead = false;

	// If true, the tormentor will ignore the player even if they are visible until the tormentor can get to their position.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Tormentor Variables")
	bool bIsIgnoringPlayer = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Tormentor Variables")
	ETormentorAttackType attackPrepared = ETormentorAttackType::SLAM;

	// The tile the tormentor last saw the player on.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Tormentor Variables")
	ADungeonSingleTile* LastPlayerSeenTile;

	// The path to the current target, with [0] being the next desired tile and Last() being the target.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Tormentor Variables")
	TArray<ADungeonSingleTile*> DesiredPath;

	// The current facing direction
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Tormentor Variables")
	ECardinal Facing = ECardinal::NORTH;
};
