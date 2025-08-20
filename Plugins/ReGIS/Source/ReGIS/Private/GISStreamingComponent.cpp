// Fill out your copyright notice in the Description page of Project Settings.


#include "GISStreamingComponent.h"

// Sets default values for this component's properties
UGISStreamingComponent::UGISStreamingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGISStreamingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGISStreamingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}



UGISStreamingComponent::AtlasStaticStreaming::AtlasStaticStreaming() 
	: AtlasStaticStreaming(2, 2, 4, 4, 256, 256) // Default constructor with default values
{
}

UGISStreamingComponent::AtlasStaticStreaming::AtlasStaticStreaming(
	int8 InCameraLengthX, int8 InCameraLengthY, 
	int8 InAtlasLengthX, int8 InAtlasLengthY, 
	int16 InTileSizeX, int16 InTileSizeY)
	: CameraTileCountX(InCameraLengthX), CameraTileCountY(InCameraLengthY), 
	AtlasTileCountX(InAtlasLengthX), AtlasTileCountY(InAtlasLengthY), 
	TileSizeX(InTileSizeX), TileSizeY(InTileSizeY),
	CameraPixelCountX(CameraTileCountX* TileSizeX),
	CameraPixelCountY(CameraTileCountY* TileSizeY),
	AtlasPixelCountX(AtlasTileCountX* TileSizeX),
	AtlasPixelCountY(AtlasTileCountY* TileSizeY),
	TilePixelCount(TileSizeX*TileSizeY),
	CameraOffsetX(0.0f), CameraOffsetY(0.0f)
{
	AtlasTileData.SetNumZeroed(AtlasPixelCountX * AtlasPixelCountY);
	StreamingTexture = UTexture2D::CreateTransient(AtlasPixelCountX, AtlasPixelCountY);
	StreamingTexture->SRGB = true; // (gamma correction) or false (linear color space) 
	//Reason: "0.5" percieved brightness ain't half max intensity 1 in reality, its close to '0.22'
	StreamingTexture->UpdateResource();
}



void UGISStreamingComponent::AtlasStaticStreaming::ExtractPixelsFromTexture(UTexture2D* Texture, TArray<FColor>& OutPixels)
{
	check(Texture);	
	if (!Texture)
	{
		UE_LOG(LogTemp, Warning, TEXT("StreamingTexture was null, skipping update"));
		return;
	}

	OutPixels.SetNumUninitialized(Texture->GetSizeX() * Texture->GetSizeY());
	FByteBulkData& BulkData =Texture->GetPlatformData()->Mips[0].BulkData;

	// Lock
	void* Data = BulkData.Lock(LOCK_READ_ONLY);
	FMemory::Memcpy(OutPixels.GetData(), Data, OutPixels.Num()*sizeof(FColor));
	// Unlock
	BulkData.Unlock();
	return;

}

void UGISStreamingComponent::AtlasStaticStreaming::CopyPixelsToAtlas(TArray<FColor> InPixels, int8 AtlasTileIndexX, int8 AtlasTileIndexY)
{ 
	const int32 TileIndex = AtlasTileIndexY * AtlasTileCountX + AtlasTileIndexX;
	FColor* AtlasTileDataPtr = AtlasTileData.GetData() + TileIndex*TilePixelCount;
	FColor* SourceDataPtr = InPixels.GetData();
	memcpy(AtlasTileDataPtr, SourceDataPtr, TilePixelCount * sizeof(FColor));
}

void UGISStreamingComponent::AtlasStaticStreaming::ConvertTileToRowContigous(
	TArray<FColor>& ContigousTileArray)
{
	TArray<FColor> TempTileData;
	TempTileData.SetNumUninitialized(AtlasPixelCountX * AtlasPixelCountY);

	const int32 TotalTiles = AtlasTileCountX * AtlasTileCountY;

	ParallelFor(TotalTiles, [&](int32 TileIndex)
		{
			const int32 TileIndexX = TileIndex % AtlasTileCountX;
			const int32 TileIndexY = TileIndex / AtlasTileCountX;

			// Even though it's entire atlas data! cuz of CONIGUENSY think of IT as JUST PURE ONE tile data 
			// Just intutive 2D array = 2D texture EXACT
			FColor* SrcTileDataPtr = ContigousTileArray.GetData() + TileIndex * TilePixelCount;

			// HERE YOU ARE JUST COPYING SINGLE TILE into AN BIG ATLAS 
			// THIS IS FOR REAL 2D array = 2D texture EXACT
			FColor* DstRowDataPtr = TempTileData.GetData()
				+ (TileIndexY * AtlasPixelCountX) * TileSizeY
				+ (TileIndexX * TileSizeX);

			for (int32 i = 0; i < TileSizeY; ++i)
			{
				const FColor* SrcRow = SrcTileDataPtr + i * TileSizeX; 
				FColor* DstRow = DstRowDataPtr + i * AtlasPixelCountX;

				FMemory::Memcpy(DstRow, SrcRow, TileSizeX * sizeof(FColor));
			}
		});

	ContigousTileArray = MoveTemp(TempTileData);
}

void UGISStreamingComponent::AtlasStaticStreaming::ConvertRowToTileContigous(TArray<FColor>& ContigousRowArray)
{
	// INTERESTING PART!
	// FUNCTION IS EXACT COPY OF ConvertTileToRowContigous
	// only memcpy is REVERSED (dest,source) to (source,dest) will make this one to other and vice versa
	//THINK OF THIS AS HAVING ONE SMALL ARRAY just enough for ONE TILE DATA
	//AND WE ARE COPYING INTO IT INTO FROM VERY BIG ARRAY (atlas)
	TArray<FColor> TempTileData;
	TempTileData.SetNumUninitialized(AtlasPixelCountX * AtlasPixelCountY);

	const int32 TotalTiles = AtlasTileCountX * AtlasTileCountY;

	ParallelFor(TotalTiles, [&](int32 TileIndex)
		{
			const int32 TileIndexX = TileIndex % AtlasTileCountX;
			const int32 TileIndexY = TileIndex / AtlasTileCountX;
			FColor* DstTileDataPtr = TempTileData.GetData() + TileIndex * TilePixelCount;
			FColor* SrcRowDataPtr = ContigousRowArray.GetData()
				+ (TileIndexY * AtlasPixelCountX) * TileSizeY
				+ (TileIndexX * TileSizeX);
			for (int32 i = 0; i < TileSizeY; ++i)
			{
				FColor* DstRow = DstTileDataPtr + i * TileSizeX;
				const FColor* SrcRow = SrcRowDataPtr + i * AtlasPixelCountX;
				FMemory::Memcpy(DstRow, SrcRow, TileSizeX * sizeof(FColor));
			}
		});
	ContigousRowArray = MoveTemp(TempTileData);
	// QUESTION: this all is ditto same as ConvertTileToRowContigous, YET IT FUCKIN WORKS WHY? 
	// there should have been BIT OF DIFFERENCE!! 
	// ANSWER - NO NEEDN'T BE & our methodf of doing multithreading is ParallelFor(TotalTiles, ... )
	// is the actual reason for structure being exact fuckin same
	// OTHER THE BIG FUCKIN GOAT REASON is JUST 
	// ONE ARRAY WAS GARBAGE, ONE WAS MEANIGFUL which was garbage which was meanigful just got reversed
}

