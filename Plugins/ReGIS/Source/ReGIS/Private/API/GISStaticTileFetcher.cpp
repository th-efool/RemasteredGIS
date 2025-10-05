#include "API/GISStaticTileFetcher.h"

#include "Utils/GISDataType.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interfaces/IHttpResponse.h"






void GISStaticTileFetcher::HandleAPIResponse(FHttpResponsePtr Response, TFunction<void(void*)> callback)
{
	GISAPIBase::HandleAPIResponse(Response, callback);
	const TArray<uint8>& ImageData = Response->GetContent();
	IImageWrapperModule& ImageWrapper = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");

	// ENCODING DATA
	TSharedPtr<IImageWrapper> Wrapper = ImageWrapper.CreateImageWrapper(EImageFormat::PNG);
	check(Wrapper->SetCompressed(ImageData.GetData(), ImageData.Num()));
	
	// DECODING DATA
	TArray<uint8> RawData;
	check(Wrapper->GetRaw(ERGBFormat::BGRA, 8, RawData)) 

	// PRPEPARING RESOURCES
	UTexture2D* Texture = UTexture2D::CreateTransient(256, 256);
	void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();

	// CALLBACK
	callback(Texture);
}



void* GISStaticTileFetcher::GetFallbackResource()
{
	/*
	static UTexture2D* FallbackTexture;
	if (FallbackTexture){return static_cast<void*>(FallbackTexture);}
	*/
	UTexture2D* FallbackTexture;
	FallbackTexture = UTexture2D::CreateTransient(256, 256, PF_B8G8R8A8);
	FallbackTexture->MipGenSettings = TMGS_NoMipmaps;
	FallbackTexture->SRGB = true;
	FColor FillColor = FColor::Black;
	FString ColorName = TEXT("Black");

	FillColor = FColor::MakeRandomColor();
	UE_LOG(LogTemp, Display, TEXT("Chosen color: %s"), *FillColor.ToString());
	
	
	TArray<FColor> Pixels;
	Pixels.Init(FillColor, 256 * 256);

	void* TextureDataBuffer = FallbackTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureDataBuffer, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
	FallbackTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

	FallbackTexture->UpdateResource();
	FallbackTexture->AddToRoot();

	return static_cast<void*>(FallbackTexture);
}

UTexture2D* GISStaticTileFetcher::GetMarkedDebugResource(FColor FillColor)
{
	UTexture2D* DebugTexture = UTexture2D::CreateTransient(256, 256, PF_B8G8R8A8);
	DebugTexture->MipGenSettings = TMGS_NoMipmaps;
	DebugTexture->SRGB = true;
	
	
	TArray<FColor> Pixels;
	Pixels.Init(FillColor, 256 * 256);

	void* TextureDataBuffer = DebugTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureDataBuffer, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
	DebugTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

	DebugTexture->UpdateResource();
	DebugTexture->AddToRoot();
	return DebugTexture;
}

void GISStaticTileFetcher::MakeApiCall(IGISCustomDatatypes& Params, TFunction<void(void*)> callback)
{
	GISAPIBase::MakeApiCall(Params, callback);
}

FString GISStaticTileFetcher::buildAPIURL(IGISCustomDatatypes& Params)
{
	ParamsStaticTileFetcher StaticTileParams = static_cast<ParamsStaticTileFetcher&>(Params);
	FString AccessToken = TEXT("pk.eyJ1IjoiYWdyaW1zaW5naHgiLCJhIjoiY21ieTk4dTk3MWtpZTJqcXVvcnVicDJhciJ9.k5ZiiC0KNvTaNIzI7uo7lA");
	UE_LOG(LogTemp, Display, TEXT("TILE ID: Zoom: %d, X: %d, Y: %d"),
		StaticTileParams.TileID.ZoomLevel, StaticTileParams.TileID.X, StaticTileParams.TileID.Y)
	
	return FString::Printf(TEXT("https://api.mapbox.com/styles/v1/mapbox/streets-v11/tiles/256/%d/%d/%d?access_token=%s"),
		StaticTileParams.TileID.ZoomLevel, StaticTileParams.TileID.X, StaticTileParams.TileID.Y, *AccessToken);
	
}


GISStaticTileFetcher::GISStaticTileFetcher()
{
}

GISStaticTileFetcher::~GISStaticTileFetcher()
{
}