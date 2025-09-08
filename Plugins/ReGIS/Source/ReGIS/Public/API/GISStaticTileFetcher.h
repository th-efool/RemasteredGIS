#pragma once
#include "GISAPIBase.h"
#include "GISDataType.h"

class GISStaticTileFetcher : GISAPIBase
{
public:
	GISStaticTileFetcher();
	virtual ~GISStaticTileFetcher();
	
private:
	virtual FString buildAPIURL(ICustomParams& Params) override;
	virtual void HandleAPIResponse(FHttpResponsePtr Response, TFunction<void(void*)> callback) override;
public: 
	virtual void* GetFallbackResource() override;

};

