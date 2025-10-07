#pragma once
#include "GISAPIBase.h"
#include "Utils/GISDataType.h"
#include "DBMS/FGISIdentifier.h"

struct ParamsStaticTileFetcher : IGISCustomDatatypes
{
	inline ParamsStaticTileFetcher(FGISTileID TileID){ this->TileID = TileID; }
	FGISTileID TileID;
};


class GISStaticTileFetcher : GISAPIBase
{
public:
	virtual void MakeApiCall(IGISCustomDatatypes& Params, TFunction<void(void*)> callback) override;

private:
	virtual FString buildAPIURL(IGISCustomDatatypes& Params) override;
	virtual void HandleAPIResponse(FHttpResponsePtr Response, TFunction<void(void*)> callback) override;
public: 
	virtual void* GetFallbackResource() override;
	static UTexture2D* GetMarkedDebugResource(FColor FillColor);
	static UTexture2D* GetLoadingTile();
	GISStaticTileFetcher();
	virtual ~GISStaticTileFetcher();
};

