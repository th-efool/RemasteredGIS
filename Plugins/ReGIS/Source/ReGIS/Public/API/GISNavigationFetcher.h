#pragma once
#include "API/GISAPIBase.h"
#include "Utils/GISNavigationDatatype.h"
#include "DBMS/FGISIdentifier.h"


class GISNavigationFetcher :  GISAPIBase
{
public:
public:
	virtual void MakeApiCall(IGISCustomDatatypes& Params, TFunction<void(void*)> callback) override;

private:
	virtual FString buildAPIURL(IGISCustomDatatypes& Params) override;
	virtual void HandleAPIResponse(FHttpResponsePtr Response, TFunction<void(void*)> callback) override;
public: 
	virtual void* GetFallbackResource() override;
	
};
