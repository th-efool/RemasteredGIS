// Fill out your copyright notice in the Description page of Project Settings.


#include "GISAPIBase.h"

#include "HttpModule.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Engine/Texture2D.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "ImageUtils.h"

GISAPIBase::GISAPIBase()
{
}

void GISAPIBase::MakeApiCall(ICustomParams& Params, TFunction<void(void*)> callback)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(buildAPIURL(Params));
	Request->SetVerb("GET");
	ActiveRequests.Add(Request);
	PendingCallbacks.Add(Request, callback);
	Request->OnProcessRequestComplete().BindLambda(
		[this,callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			
		});
	Request->ProcessRequest();
}

void GISAPIBase::HandleAPIResponse(FHttpResponsePtr Response, TFunction<void(void*)> callback)
{
}

void GISAPIBase::OnAPIResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
	TFunction<void(void*)>*  CallbackPtr = PendingCallbacks.Find(Request);
	check(CallbackPtr);
	if (!(CallbackPtr)){return;}
	TFunction<void(void*)> Callback = MoveTemp(*CallbackPtr);
	PendingCallbacks.Remove(Request);
	ActiveRequests.Remove(Request.ToSharedRef());

	if (!this || !bSuccess || !Response.IsValid())
	{Callback(GetFallbackResource());}
	else 
	{HandleAPIResponse(Response,Callback);};
}

GISAPIBase::~GISAPIBase()
{
	CancelAllRequests();
}

void GISAPIBase::CancelAllRequests()
{
	for (TSharedRef<IHttpRequest>& Request : ActiveRequests)
	{
		if (Request->GetStatus() == EHttpRequestStatus::Processing)
		{Request->CancelRequest();}
	}
	ActiveRequests.Empty();
}