#include "D:\ThisPC\Documents\Unreal Projects\RemasteredGIS\Intermediate\Build\Win64\x64\RemasteredGISEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.ValExpApi.Cpp20.h"
#include "GISStaticTileFetcher.h"



struct ParamsStaticTileFetcher : ICustomParams
{
	FGISTileID TileID;
};


struct ParamIStaticTileDecode : ICustomParams
{
	
};

struct ParamIStaticTileEncode : ICustomParams
{
	
};

void GISStaticTileFetcher::MakeApiCall(ICustomParams& Params, TFunction<void(void*)> callback)
{
	
}

void GISStaticTileFetcher::Decode(ICustomParams Params)
{
	
}

void GISStaticTileFetcher::Encode(ICustomParams Params)
{
}

void* GISStaticTileFetcher::FallbackResponse()
{
	return nullptr;
}

FString GISStaticTileFetcher::buildAPIURL(ICustomParams& Params)
{
	ParamsStaticTileFetcher StaticTileParams = static_cast<ParamsStaticTileFetcher&>(Params);
	FString AccessToken = TEXT("pk.eyJ1IjoiYWdyaW1zaW5naHgiLCJhIjoiY21ieTk4dTk3MWtpZTJqcXVvcnVicDJhciJ9.k5ZiiC0KNvTaNIzI7uo7lA"); 
	return FString::Printf(TEXT("https://api.mapbox.com/styles/v1/mapbox/streets-v11/tiles/256/%d/%d/%d?access_token=%s"),
		StaticTileParams.TileID.ZoomLevel, StaticTileParams.TileID.X, StaticTileParams.TileID.Y, *AccessToken);

}
