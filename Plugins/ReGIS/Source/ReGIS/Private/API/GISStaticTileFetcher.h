#pragma once
#include "GISAPIBase.h"
#include "GISDataType.h"

class GISStaticTileFetcher : GISAPIBase
{
public:
	GISStaticTileFetcher();
	virtual void MakeApiCall(ICustomParams& Params, TFunction<void(void*)> callback) override;
	virtual void Decode(ICustomParams Params) override;
	virtual void Encode(ICustomParams Params) override;
	virtual void* GetFallbackResource() override;
	virtual FString buildAPIURL(ICustomParams& Params) override;
	virtual void HandleAPIResponse(FHttpResponsePtr Response, TFunction<void(void*)> callback) override;
};

