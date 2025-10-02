// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GISTileViewportActor.h"
#include  "Utils/GISDataType.h"
#include "Streaming/StaticStreaming.h"
#include "GISStaticTileViewportActor.generated.h"


/**
 * 
 */
UCLASS()
class REGIS_API AGISStaticTileViewportActor : public AGISTileViewportActor
{
	GENERATED_BODY()
	
public:
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	unsigned int FetchIndex=0;
	void FetchVisibleTiles();
	void HandleTexture(UTexture2D* Texture,unsigned int fetchIndex, int TileIndex) const;
	
	// STREAMER CONSTRUCTION
	StaticStreaming* StaticStreamer;
	void InitStaticStreaming();
	
// INPUTS & CONSTRUCTION
public:
	virtual void RefreshConfig() override;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;    
	UPROPERTY()
	UStaticMeshComponent* TileMesh;
	AGISStaticTileViewportActor();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")    
	FGISStreamingConfig InStreamingConfig;  
	  
	FGISTileID CenterTileID;  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")    
	int ZoomLevel = 14;    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")    
	int CenterX = 100;    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")    
	int CenterY = 100;  

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* TileBaseMeshAsset;    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")    
	UMaterialInterface* TileBaseMaterialAsset;

private:
	void TestCameraMovement();
}; 
