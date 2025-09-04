// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GISDataType.h"
#include "HttpFwd.h"
class IHttpRequest;
/**
 * 
 */
class GISAPIBase
{
public:
	GISAPIBase();
	virtual ~GISAPIBase();

	void MakeApiCall(ICustomParams& Params, TFunction<void(void*)> callback);
	TArray<TSharedRef<IHttpRequest>> ActiveRequests;
	TMap<FHttpRequestPtr,TFunction<void(void*)> > PendingCallbacks;

	virtual void* GetFallbackResource() =0; // create an static Fallback resource variable

	
	protected:
	virtual FString buildAPIURL(ICustomParams& Params)=0;

	virtual void HandleAPIResponse(FHttpResponsePtr Response, TFunction<void(void*)> callback);


private:
	void OnAPIResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
	void CancelAllRequests();

};

