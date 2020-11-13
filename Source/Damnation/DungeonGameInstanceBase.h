// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Slate/Public/Slate.h"
#include "MoviePlayer/Public/MoviePlayer.h"
#include "DungeonGameInstanceBase.generated.h"

/**
 * 
 */
UCLASS()
class DAMNATION_API UDungeonGameInstanceBase : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	virtual void BeginLoadingScreen(const FString& InMapName);
	UFUNCTION(BlueprintCallable)
	virtual void EndLoadingScreen(UWorld* InLoadedWorld);

	UPROPERTY(EditAnywhere)
	UTexture2D* LoadingScreenImage;
	UPROPERTY(EditAnywhere)
	UTexture2D* LoadingScreenBackground;
	UPROPERTY(EditAnywhere)
	float LoadingScreenIconRotateSpeed;
	UPROPERTY(EditAnywhere)
	float LoadingScreenIconEdgeSize;
};
