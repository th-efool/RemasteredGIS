// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GISTileViewportActor.h"
#include  "Utils/GISDataType.h"
#include "Streaming/StaticStreaming.h"
#include "Utils/GISConversionEngine.h"
#include <utility>
#include "GISStaticTileViewportActor.generated.h"


/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCanvasClicked, FVector2D, LocalCoord);


USTRUCT(BlueprintType)   // <-- makes struct visible/usable in BP
struct FInputTileData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile")
	int ZoomLevel = 14;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile")
	bool UseLatitudeLongitude = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile")
	int CenterX = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile")
	int CenterY = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile")
	double Latitude = 28.61;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile")
	double Longitude = 77.23;
};


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
	FGISTileID CenterTileID;  

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")    
	FGISStreamingConfig InStreamingConfig;  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FInputTileData InputConfigData;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* TileBaseMeshAsset;    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")    
	UMaterialInterface* TileBaseMaterialAsset;

private:
	void TestCameraMovement();
public:
	UPROPERTY(BlueprintAssignable, Category="Canvas")
	FOnCanvasClicked OnCanvasClicked;
	UFUNCTION()
	void HandleOnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
	
private:
	FGISPoint ConvertLocalPointToGISPoint(FVector2D LocalCoord) const;
	
};
