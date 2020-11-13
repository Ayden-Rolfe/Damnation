// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ProjectileInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UProjectileInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DAMNATION_API IProjectileInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Execute projectile functionality. Return value determines if the projectile should be destroyed.
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	bool ProjectileAction();
};
