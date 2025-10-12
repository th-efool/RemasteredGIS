// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Trees.h"
#include "FGISIdentifier.h"
#include "API/GISStaticTileFetcher.h"
#include "API/GISNavigationFetcher.h"
#include "DataManager.generated.h"

class IBaseGISDataResource;


UCLASS()
class REGIS_API UDataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	
	UDataManager();
private:
	void InitializeComponents();

	// StaticTile
public:
	inline QuadTree* GetStaticTileQuadTree(){ return StaticTileQT;};
	void GetStaticTile(FGISTileID TileID, TFunction<void(UTexture2D*)> callback);
private:
	GISStaticTileFetcher* StaticTileFetcher;
	QuadTree* StaticTileQT;
	TMap<uint64, TArray<TFunction<void(UTexture2D*)>>> PendingCallbacks;
	
	// Navigation
public:
	void GetNavigationData(ParamsNavigationFetcher NavParams, TFunction<void(FRoute*)> callback);
	TMap<uint64, TFunction<void(FRoute*)>> PendingNavigationCallbacks;

private:
	GISNavigationFetcher* NavigationFetcher;
};




