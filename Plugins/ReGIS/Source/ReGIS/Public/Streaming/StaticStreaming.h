#pragma once
class GISStaticTileFetcher;

class StaticStreaming
{
public:
	// Construction
	StaticStreaming(int8 InCameraLengthX, int8 InCameraLengthY,
					int8 InAtlasLengthX, int8 InAtlasLengthY,
					int16 InTileSizeX, int16 InTileSizeY);
	// ---- External Inputs ----
	// 1) Set which tiles are visible (game provides arrya of UTexture2D*)
	void SetVisibleTiles(const TArray<UTexture2D*>& InTiles);
	void SetVisibleTilesToFallback();

	void ReInitVisibleTiles();
	void SetVisibleTileIndexed(UTexture2D* InTile, int Index);
	// 2) Set camera offset (normalized -1..1 range, or pixels if you prefer)
	void SetCameraOffset(float OffsetX, float OffsetY);
	
	// ---- Object Output Accessors ----
	UTexture2D* GetStreamingTexture() const { return StreamingTexture; }


public:
	// ---- Triggers ----
	// 1) Rebuild/update atlas after visible tiles changed
	void UpdateAtlas();
	// 2) Update streaming texture (uses last SetCameraOffset)
	void UpdateStreaming();

private:
	// external data storage
	TArray<UTexture2D*> VisibleTiles;
	
	// internal
	TArray<FColor> AtlasTileData;
	UTexture2D* StreamingTexture;


	float CameraOffsetX = 0.1f;
	float CameraOffsetY = 0.1f;

	// ---- Private helpers ----
	void ExtractPixelsFromTexture(UTexture2D* Texture, TArray<FColor>& OutPixels);
	void CopyPixelsToAtlasTileContiguous(TArray<FColor> InPixels,
										 int8 AtlasTileIndexX, int8 AtlasTileIndexY);


	static void ConvertTileArrayToRowContiguous(TArray<FColor>& ContigousTileArray, int InAtlasPixelCountX, int InAtlasPixelCountY, int8 InAtlasTileCountX, int InAtlasTileCountY, int16 InTileSizeX, int16 InTileSizeY, int InTilePixelCount);
	static void ConvertRowArrayToTileContiguous(TArray<FColor>& ContigousRowArray, int InAtlasPixelCountX, int InAtlasPixelCountY, int8 InAtlasTileCountX, int InAtlasTileCountY, int16 InTileSizeX, int16 InTileSizeY, int InTilePixelCount);
	;
public:
	// ---- Initialization Internal state ----
	int8 CameraTileCountX, CameraTileCountY;
	int8 AtlasTileCountX, AtlasTileCountY;
	int16 TileSizeX, TileSizeY;

	int CameraPixelCountX, CameraPixelCountY;
	int AtlasPixelCountX, AtlasPixelCountY;
	int TilePixelCount;

private:
	// Fetches tiles and fills it for two purposes
	// 1. Easier Debug
	// 2. Prevent Unneccesary Crash
	GISStaticTileFetcher* InitFallbackStaticTileFetcher;

	
};



