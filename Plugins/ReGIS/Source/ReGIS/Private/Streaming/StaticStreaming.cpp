#include "Streaming/StaticStreaming.h"

#include "Utils/GISErrorHandler.h"
#include "API/GISStaticTileFetcher.h"


/*StaticStreaming::StaticStreaming() 
	: StaticStreaming(2, 2, 4, 4, 256, 256) // Default constructor with default values
{
}*/

StaticStreaming::StaticStreaming(
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
	TilePixelCount(TileSizeX*TileSizeY)  
{
	AtlasTileData.SetNumZeroed(AtlasPixelCountX * AtlasPixelCountY);
	StreamingTexture = UTexture2D::CreateTransient(CameraPixelCountX, CameraPixelCountY);
	StreamingTexture->SRGB = true; // (gamma correction) or false (linear color space) 
	//Reason: "0.5" percieved brightness ain't half max intensity 1 in reality, its close to '0.22'
	StreamingTexture->UpdateResource();
	
	InitFallbackStaticTileFetcher= new GISStaticTileFetcher();
	SetVisibleTilesToFallback();
	UpdateStreaming();
	
}

void StaticStreaming::SetVisibleTiles(const TArray<UTexture2D*>& InTiles)
{
	VisibleTiles = InTiles;
	UpdateAtlas();
}

void StaticStreaming::SetVisibleTilesToFallback()
{
	VisibleTiles.Empty(); // reset before filling
	int mid = (AtlasTileCountX - 1) / 2;          // 0-based row/col
	int centerIndex = mid * AtlasTileCountX + mid + 1; // 1-based sequential index
	centerIndex-=1; //Counting Starts From Zero
	
	for (int32 i = 0; i < AtlasTileCountX*AtlasTileCountY; i++)
	{
		if (i==centerIndex)
		{
			VisibleTiles.Add(static_cast<UTexture2D*>(InitFallbackStaticTileFetcher->GetMarkedDebugResource(FColor::Black)));
		} 
		else
		{
			VisibleTiles.Add(static_cast<UTexture2D*>(InitFallbackStaticTileFetcher->GetMarkedDebugResource(FColor::Purple)));
		}
	}
	UpdateAtlas();

}

void StaticStreaming::ReInitVisibleTiles()
{
	VisibleTiles.Empty();
	for (int32 i = 0; i < AtlasTileCountX*AtlasTileCountY; i++)
	{
		VisibleTiles.Add(static_cast<UTexture2D*>(InitFallbackStaticTileFetcher->GetLoadingTile()));
	}
}

void StaticStreaming::SetVisibleTileIndexed(UTexture2D* InTile, int Index)
{
	VisibleTiles[Index] = InTile;
	UE_LOG(LogTemp, Display, TEXT("Succesful FetchbackOf: %d"), Index)
	UpdateAtlas();
	UpdateStreaming();
}


void StaticStreaming::SetCameraOffset(float OffsetX, float OffsetY)
{
	InCameraStreamOffsetX  = OffsetX>1 ? 1 : OffsetX;
	InCameraStreamOffsetY = OffsetY>1 ? 1 : OffsetY;
	InCameraStreamOffsetX  = OffsetX<-1 ? 1 : OffsetX;
	InCameraStreamOffsetY = OffsetY<-1 ? 1 : OffsetY;
	UpdateStreaming();
}

void StaticStreaming::UpdateAtlas()
{
	GIS_HANDLE(VisibleTiles.Num() == AtlasTileCountX*AtlasTileCountY,FLoggerLevel::Warn);
	
	
	for (int TileIndex = VisibleTiles.Num()-1; TileIndex>=0; TileIndex-- )
	{
		TArray<FColor> OutTileData;
		ExtractPixelsFromTexture(VisibleTiles[TileIndex],OutTileData);
		CopyPixelsToAtlasTileContiguous(OutTileData, TileIndex%AtlasTileCountX, TileIndex/AtlasTileCountX);
	};
	
	ConvertTileArrayToRowContiguous(AtlasTileData, AtlasPixelCountX,
		AtlasPixelCountY, AtlasTileCountX, AtlasTileCountY,
		TileSizeX, TileSizeY, TilePixelCount );
}



void StaticStreaming::UpdateStreaming()
{
    if (!StreamingTexture || !StreamingTexture->GetPlatformData() || StreamingTexture->GetPlatformData()->Mips.Num() == 0)
        return; 
        
	GIS_FATAL_MSG(InCameraStreamOffsetX<= 1 || InCameraStreamOffsetX>= -1 || InCameraStreamOffsetY<=1 || InCameraStreamOffsetY>=-1 ,
	FString::Printf(TEXT("Pixel Offset exceededed max offset! X=%f Y=%f"),
	 InCameraStreamOffsetX, InCameraStreamOffsetY));
	
	int MaxPixelOffsetY = (AtlasPixelCountY-CameraPixelCountY)*0.5;
	int MaxPixelOffsetX = (AtlasPixelCountX-CameraPixelCountX)*0.5;

	int DeviationX = InCameraStreamOffsetX*MaxPixelOffsetX;
	int DeviationY = InCameraStreamOffsetY*MaxPixelOffsetY;
	
	const int CenterPixelY = AtlasPixelCountY/2;
	const int CenterPixelX = AtlasPixelCountX/2;

	const int CenteredCameraFrameTopLeftPixelX = CenterPixelX - CameraPixelCountX*0.5;
	const int CenteredCameraFrameTopLeftPixelY = CenterPixelY - CameraPixelCountY*0.5;

	
	int BeginXPixel = CenteredCameraFrameTopLeftPixelX + DeviationX;
	int BeginYPixel = CenteredCameraFrameTopLeftPixelY + DeviationY;

	GIS_HANDLE_IF(BeginXPixel >= 0 || BeginXPixel <= AtlasPixelCountX)
	GIS_HANDLE_IF(BeginYPixel >= 0 || BeginYPixel <= AtlasPixelCountY)
	
	GIS_FATAL_MSG(BeginXPixel + CameraPixelCountX <= AtlasPixelCountX,
	FString::Printf(TEXT("Atlas bounds exceeded! BeginXPixel=%d CameraPixelCountX=%d AtlasPixelCountX=%d"),
	BeginXPixel, CameraPixelCountX, AtlasPixelCountX));
	
	GIS_FATAL_MSG(BeginYPixel + CameraPixelCountY <= AtlasPixelCountY,
	FString::Printf(TEXT("Atlas bounds exceeded! BeginXPixel=%d CameraPixelCountX=%d AtlasPixelCountX=%d"),
	BeginXPixel, CameraPixelCountY, AtlasPixelCountY));
	
	TArray<FColor> TempStreamingData;
	TempStreamingData.SetNumUninitialized(AtlasPixelCountX * AtlasPixelCountY);


	for (int i = BeginYPixel; (i - BeginYPixel) < CameraPixelCountY; ++i)
	{
		FColor* SrcDataPtr = AtlasTileData.GetData() + i * AtlasPixelCountX + BeginXPixel;
		FColor* DstDataPtr = TempStreamingData.GetData() + (i-BeginYPixel) * CameraPixelCountX;
		memcpy(DstDataPtr,SrcDataPtr,CameraPixelCountX*sizeof(FColor) );
	}

	void* mipBuffer = StreamingTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	memcpy(mipBuffer,TempStreamingData.GetData(),CameraPixelCountX*CameraPixelCountY*sizeof(FColor));
	StreamingTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	StreamingTexture->UpdateResource();

	double ReNormalizedX = MaxPixelOffsetX > 0 ? static_cast<double>(DeviationX) / MaxPixelOffsetX : 0.0;
	double ReNormalizedY = MaxPixelOffsetY > 0 ? static_cast<double>(DeviationY) / MaxPixelOffsetY : 0.0;

	OutputCameraOffset = FVector2D(ReNormalizedX, ReNormalizedY);

}



void StaticStreaming::ExtractPixelsFromTexture(UTexture2D* Texture, TArray<FColor>& OutPixels)
{
	GIS_FATAL_MSG(Texture, TEXT("StreamingTexture was null, skipping update"));	
	
	OutPixels.SetNumUninitialized(Texture->GetSizeX() * Texture->GetSizeY());
	FByteBulkData& BulkData =Texture->GetPlatformData()->Mips[0].BulkData;

	// Lock
	void* Data = BulkData.Lock(LOCK_READ_ONLY);
	FMemory::Memcpy(OutPixels.GetData(), Data, OutPixels.Num()*sizeof(FColor));
	// Unlock
	BulkData.Unlock();
	return;

}

void StaticStreaming::CopyPixelsToAtlasTileContiguous(TArray<FColor> InPixels, int8 AtlasTileIndexX, int8 AtlasTileIndexY)
{ 
	const int32 TileIndex = AtlasTileIndexY * AtlasTileCountX + AtlasTileIndexX;
	FColor* AtlasTileDataPtr = AtlasTileData.GetData() + TileIndex*TilePixelCount;
	FColor* SourceDataPtr = InPixels.GetData();
	memcpy(AtlasTileDataPtr, SourceDataPtr, TilePixelCount * sizeof(FColor));
}

void StaticStreaming::ConvertTileArrayToRowContiguous(
	TArray<FColor>& ContigousTileArray, int InAtlasPixelCountX, int InAtlasPixelCountY, int8 InAtlasTileCountX, int InAtlasTileCountY, int16 InTileSizeX, int16 InTileSizeY, int InTilePixelCount )
{
	TArray<FColor> TempTileData;
	TempTileData.SetNumUninitialized(InAtlasPixelCountX * InAtlasPixelCountY);

	const int32 TotalTiles = InAtlasTileCountX * InAtlasTileCountY;

	ParallelFor(TotalTiles, [&](int32 TileIndex)
		{
			const int32 TileIndexX = TileIndex % InAtlasTileCountX;
			const int32 TileIndexY = TileIndex / InAtlasTileCountX;

			// Even though it's entire atlas data! cuz of CONIGUENSY think of IT as JUST PURE ONE tile data 
			// Just intutive 2D array = 2D texture EXACT
			FColor* SrcTileDataPtr = ContigousTileArray.GetData() + TileIndex * InTilePixelCount;

			// HERE YOU ARE JUST COPYING SINGLE TILE into AN BIG ATLAS 
			// THIS IS FOR REAL 2D array = 2D texture EXACT
			FColor* DstRowDataPtr = TempTileData.GetData()
				+ (TileIndexY * InAtlasPixelCountX) * InTileSizeY
				+ (TileIndexX * InTileSizeX);

			for (int32 i = 0; i < InTileSizeY; ++i)
			{
				const FColor* SrcRow = SrcTileDataPtr + i * InTileSizeX; 
				FColor* DstRow = DstRowDataPtr + i * InAtlasPixelCountX;

				FMemory::Memcpy(DstRow, SrcRow, InTileSizeX * sizeof(FColor));
			}
		});

	ContigousTileArray = MoveTemp(TempTileData);
}

void StaticStreaming::ConvertRowArrayToTileContiguous(TArray<FColor>& ContigousRowArray, int InAtlasPixelCountX, int InAtlasPixelCountY, int8 InAtlasTileCountX, int InAtlasTileCountY, int16 InTileSizeX, int16 InTileSizeY, int InTilePixelCount)
{
	// INTERESTING PART!
	// FUNCTION IS EXACT COPY OF ConvertTileToRowContigous
	// only memcpy is REVERSED (dest,source) to (source,dest) will make this one to other and vice versa
	//THINK OF THIS AS HAVING ONE SMALL ARRAY just enough for ONE TILE DATA
	//AND WE ARE COPYING INTO IT INTO FROM VERY BIG ARRAY (atlas)
	TArray<FColor> TempTileData;
	TempTileData.SetNumUninitialized(InAtlasPixelCountX * InAtlasPixelCountY);

	const int32 TotalTiles = InAtlasTileCountX * InAtlasTileCountY;

	ParallelFor(TotalTiles, [&](int32 TileIndex)
		{
			const int32 TileIndexX = TileIndex % InAtlasTileCountX;
			const int32 TileIndexY = TileIndex / InAtlasTileCountX;
			FColor* DstTileDataPtr = TempTileData.GetData() + TileIndex * InTilePixelCount;
			FColor* SrcRowDataPtr = ContigousRowArray.GetData()
				+ (TileIndexY * InAtlasPixelCountX) * InTileSizeY
				+ (TileIndexX * InTileSizeX);
			for (int32 i = 0; i < InTileSizeY; ++i)
			{
				FColor* DstRow = DstTileDataPtr + i * InTileSizeX;
				const FColor* SrcRow = SrcRowDataPtr + i * InAtlasPixelCountX;
				FMemory::Memcpy(DstRow, SrcRow, InTileSizeX * sizeof(FColor));
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