// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GISDataType.h"
#include "Components/ActorComponent.h"
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
		int8 CameraTileCountX; 
		int8 CameraTileCountY;
		int8 AtlasTileCountX;
		int8 AtlasTileCountY; 
		int16 TileSizeX = 256; // Size of each tile in pixels
		int16 TileSizeY = 256;
		int CameraPixelCountX;
		int CameraPixelCountY;
		int AtlasPixelCountX;
		int AtlasPixelCountY;
		int TilePixelCount;
		TArray<FColor> AtlasTileData; // Tile data for the atlas
		UTexture2D* StreamingTexture; // Texture for the atlas


		// Atlas Build & Update methods
		void BuildAtlas();
		void UpdateAtlas();
		//Atlas related helper methods
		void ExtractPixelsFromTexture(UTexture2D* Texture, TArray<FColor>& OutPixels);
		void CopyPixelsToAtlas(TArray<FColor> InPixels, int8 AtlasTileIndexX, int8 AtlasTileIndexY);
		void ConvertTileToRowContigous(TArray<FColor>& ContigousTileArray);
		void ConvertRowToTileContigous(TArray<FColor>& ContigousRowArray);


		// Texture Build & Update methods
		float CameraOffsetX;
		float CameraOffsetY;
		void BuildStreaming();
		void UpdateStreaming();


	};

public:	
	// Sets default values for this component's properties
	UGISStreamingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
