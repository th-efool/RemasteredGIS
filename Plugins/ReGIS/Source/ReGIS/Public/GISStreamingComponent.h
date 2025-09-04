// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GISDataType.h"
#include "Components/ActorComponent.h"
#include "API/GISStaticTileFetcher.h"
#include "GISStreamingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REGIS_API UGISStreamingComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StreamingConfig")
	FGISStreamingConfig InStreamingConfig;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StreamingConfig")
	FGISTileID InCenterTile;
	
	
	class AtlasStaticStreaming {
	public :

		// Constructor & Initializatiors 
		AtlasStaticStreaming();
		AtlasStaticStreaming(int8 InCameraLengthX, int8 InCameraLengthY, int8 InAtlasLengthX, int8 InAtlasLengthY, int16 TileSizeX, int16 TileSizeY);
		int8 CameraTileCountX=3; 
		int8 CameraTileCountY=3;
		int8 AtlasTileCountX=7;
		int8 AtlasTileCountY=7; 
		int16 TileSizeX = 256; // Size of each tile in pixels
		int16 TileSizeY = 256;
		int CameraPixelCountX;
		int CameraPixelCountY;
		int AtlasPixelCountX;
		int AtlasPixelCountY;
		int TilePixelCount;
		TArray<UTexture2D*> VisibleTiles;

		TArray<FColor> AtlasTileData; // Tile data for the atlas
		UTexture2D* StreamingTexture; // Camera Texture Cutout for the atlas
		GISStaticTileFetcher* StaticTileFetcher;

		// Atlas Build & Update methods
		void BuildUpdateAtlas();
		//Atlas related helper methods
		void ExtractPixelsFromTexture(UTexture2D* Texture, TArray<FColor>& OutPixels);
		void CopyPixelsToAtlasTileContiguous(TArray<FColor> InPixels, int8 AtlasTileIndexX, int8 AtlasTileIndexY);
		
		static void ConvertTileArrayToRowContiguous(TArray<FColor>& ContigousTileArray, int InAtlasPixelCountX, int InAtlasPixelCountY, int8 InAtlasTileCountX, int InAtlasTileCountY, int16 InTileSizeX, int16 InTileSizeY, int InTilePixelCount);
		static void ConvertRowArrayToTileContiguous(TArray<FColor>& ContigousRowArray, int InAtlasPixelCountX, int InAtlasPixelCountY, int8 InAtlasTileCountX, int InAtlasTileCountY, int16 InTileSizeX, int16 InTileSizeY, int InTilePixelCount);

		// Texture Build & Update methods
		void BuildUpdateStreaming(float CameraOffsetX, float CameraOffsetY);
	};

public:	
	// Sets default values for this component's properties
	UGISStreamingComponent();

public:
	AtlasStaticStreaming* StaticStreaming;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void InitStreaming();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
};
