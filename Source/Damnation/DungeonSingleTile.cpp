// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonSingleTile.h"

// Sets default values
ADungeonSingleTile::ADungeonSingleTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
#ifdef DUNGEON_DRAWCONNECTIONS
	PrimaryActorTick.bCanEverTick = true;
#else
	PrimaryActorTick.bCanEverTick = false;
#endif // DUNGEON_DRAWCONNECTIONS

	CardinalConnections.Init(nullptr, 4);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	//DEBUG_IndexText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DEBUG Index Text"));
	//DEBUG_IndexText->SetupAttachment(RootComponent);
	//DEBUG_IndexText->SetRelativeRotation(FQuat(FRotator(0, 180, 0)));
	//DEBUG_IndexText->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
	//DEBUG_IndexText->SetTextRenderColor(FColor::Black);
}

// Called when the game starts or when spawned
void ADungeonSingleTile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADungeonSingleTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ADungeonSingleTile::CheckSurroundingTiles()
{
	for (int i = 0; i < 4; ++i)
	{
		if (!CardinalConnections[i] || CardinalConnections[i]->bPathingIgnore)
			return false;

		if (i % 2 == 0)
			if (!CardinalConnections[i]->CardinalConnections[1] ||
				!CardinalConnections[i]->CardinalConnections[3] ||
				CardinalConnections[i]->CardinalConnections[1]->bPathingIgnore ||
				CardinalConnections[i]->CardinalConnections[3]->bPathingIgnore
				)
				return false;
	}
	return true;
}

ADungeonSingleTile* ADungeonSingleTile::GetConnectedTile(ECardinal Direction)
{
	return CardinalConnections[(uint8)Direction];
}

void ADungeonSingleTile::GetSurroundingTiles(TArray<ADungeonSingleTile*>& tiles)
{
	tiles.Empty(9);
	// Trackers for tiles outside of this tiles' direct influence
	ADungeonSingleTile* tl = nullptr;
	ADungeonSingleTile* tr = nullptr;
	ADungeonSingleTile* bl = nullptr;
	ADungeonSingleTile* br = nullptr;

	for (int i = 0; i < 4; ++i)
	{
		auto current = CardinalConnections[i];

		tiles.Add(current);

		if (current)
		{
			if (i % 2 == 0)
			{
				ADungeonSingleTile* left = current->CardinalConnections[3];
				ADungeonSingleTile* right = current->CardinalConnections[1];
				if (i == 0)
				{
					tl = tl ? tl : left;
					tr = tr ? tr : right;
				}
				else
				{
					bl = bl ? bl : left;
					br = br ? br : right;
				}
			}
			else if (i % 2 != 0)
			{
				ADungeonSingleTile* up = current->CardinalConnections[0];
				ADungeonSingleTile* down = current->CardinalConnections[2];
				if (i == 1)
				{
					tr = tr ? tr : up;
					br = br ? br : down;
				}
				else
				{
					tl = tl ? tl : up;
					bl = bl ? bl : down;
				}
			}
		}
	}
	tiles.Add(tl);
	tiles.Add(tr);
	tiles.Add(br);
	tiles.Add(bl);
}

TArray<ECardinal> ADungeonSingleTile::MakeRandDirectionArray()
{
	TArray<ECardinal> arr;
	arr.Init(ECardinal::NULLDIR, 4);
	for (uint8 i = 0; i < 4; ++i)
		arr[i] = (ECardinal)i;
	ShuffleArray<ECardinal>(arr);
	return arr;
}

ADungeonSingleTile* ADungeonSingleTile::GetAdjacent(ECardinal direction)
{
	return CardinalConnections[(uint32)direction];
}

