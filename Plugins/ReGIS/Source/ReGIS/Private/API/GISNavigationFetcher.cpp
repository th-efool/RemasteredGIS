
#include "API/GISNavigationFetcher.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Interfaces/IHttpResponse.h"

void GISNavigationFetcher::MakeApiCall(IGISCustomDatatypes& Params, TFunction<void(void*)> callback)
{
	GISAPIBase::MakeApiCall(Params, callback);
}

FString GISNavigationFetcher::buildAPIURL(IGISCustomDatatypes& Params)
{
	const ParamsNavigationFetcher& NavParams = static_cast<const ParamsNavigationFetcher&>(Params);

	FString ProfileString;
	switch (NavParams.ProfileType)
	{
	case ENavigationProfile::Driving:         ProfileString = TEXT("driving"); break;
	case ENavigationProfile::Walking:         ProfileString = TEXT("walking"); break;
	case ENavigationProfile::Cycling:         ProfileString = TEXT("cycling"); break;
	case ENavigationProfile::DrivingTraffic:  ProfileString = TEXT("driving-traffic"); break;
	default:                                  ProfileString = TEXT("driving"); break;
	}

	FString AccessToken = TEXT("pk.eyJ1IjoiYWdyaW1zaW5naHgiLCJhIjoiY21ieTk4dTk3MWtpZTJqcXVvcnVicDJhciJ9.k5ZiiC0KNvTaNIzI7uo7lA");

	// Use %.15f for full double precision
	FString Start = FString::Printf(TEXT("%.15f,%.15f"), NavParams.StartLongitude, NavParams.StartLatitude);
	FString End   = FString::Printf(TEXT("%.15f,%.15f"), NavParams.EndLongitude, NavParams.EndLatitude);

	return FString::Printf(
		TEXT("https://api.mapbox.com/directions/v5/mapbox/%s/%s;%s?alternatives=%s&geometries=geojson&language=%s&overview=%s&steps=%s&access_token=%s"),
		*ProfileString,
		*Start,
		*End,
		NavParams.bAlternatives ? TEXT("true") : TEXT("false"),
		*NavParams.Language,
		*NavParams.Overview,
		NavParams.bSteps ? TEXT("true") : TEXT("false"),
		*AccessToken
	);
}

void GISNavigationFetcher::HandleAPIResponse(FHttpResponsePtr Response, TFunction<void(void*)> callback)
{
	GISAPIBase::HandleAPIResponse(Response, callback);

    if (!Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid HTTP response."));
		return;
	}

	const FString JsonString = Response->GetContentAsString();
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response"));
		return;
	}

	// Create route object on heap
	FRoute* Route = new FRoute();

	const TArray<TSharedPtr<FJsonValue>>* RoutesArray;
	if (JsonObject->TryGetArrayField(TEXT("routes"), RoutesArray) && RoutesArray->Num() > 0)
	{
		const TSharedPtr<FJsonObject> RouteObj = (*RoutesArray)[0]->AsObject();

		Route->Distance = RouteObj->GetNumberField(TEXT("distance"));
		Route->Duration = RouteObj->GetNumberField(TEXT("duration"));

		// --- Parse FULL GEOMETRY ---
		const TSharedPtr<FJsonObject>* GeometryObj;
		if (RouteObj->TryGetObjectField(TEXT("geometry"), GeometryObj))
		{
			const TArray<TSharedPtr<FJsonValue>>* CoordinatesArray;
			if ((*GeometryObj)->TryGetArrayField(TEXT("coordinates"), CoordinatesArray))
			{
				for (const TSharedPtr<FJsonValue>& CoordVal : *CoordinatesArray)
				{
					const TArray<TSharedPtr<FJsonValue>> Coord = CoordVal->AsArray();
					if (Coord.Num() >= 2)
					{
						GeoCoordinate GeoPoint;
						GeoPoint.Longitude = Coord[0]->AsNumber();
						GeoPoint.Latitude  = Coord[1]->AsNumber();
						Route->Geometry.Add(GeoPoint);
					}
				}
			}
		}

		// --- Parse steps (basic info only, no geometry) ---
		const TArray<TSharedPtr<FJsonValue>>* LegsArray;
		if (RouteObj->TryGetArrayField(TEXT("legs"), LegsArray))
		{
			for (const TSharedPtr<FJsonValue>& LegVal : *LegsArray)
			{
				const TSharedPtr<FJsonObject> LegObj = LegVal->AsObject();

				const TArray<TSharedPtr<FJsonValue>>* StepsArray;
				if (LegObj->TryGetArrayField(TEXT("steps"), StepsArray))
				{
					for (const TSharedPtr<FJsonValue>& StepVal : *StepsArray)
					{
						const TSharedPtr<FJsonObject> StepObj = StepVal->AsObject();
						FRouteStep Step;

						const TSharedPtr<FJsonObject>* SubGeometryObj;

						if (StepObj->TryGetObjectField(TEXT("geometry"), SubGeometryObj))
						{
							const TArray<TSharedPtr<FJsonValue>>* SubCoordinatesArray;
							if ((*SubGeometryObj)->TryGetArrayField(TEXT("coordinates"), SubCoordinatesArray))
							{
								for (const TSharedPtr<FJsonValue>& CoordVal : *SubCoordinatesArray)
								{
									const TArray<TSharedPtr<FJsonValue>> Coord = CoordVal->AsArray();
									if (Coord.Num() >= 2)
									{
										GeoCoordinate GeoPoint;
										GeoPoint.Longitude = Coord[0]->AsNumber();
										GeoPoint.Latitude  = Coord[1]->AsNumber();
										Step.Geometry.Add(GeoPoint);
									}
								}  
							}
						}


						
						if (const TSharedPtr<FJsonObject>* ManeuverObj;
							StepObj->TryGetObjectField(TEXT("maneuver"), ManeuverObj))
						{
							const TArray<TSharedPtr<FJsonValue>>* LocationArray;
							if ((*ManeuverObj)->TryGetArrayField(TEXT("location"), LocationArray)
								&& LocationArray->Num() >= 2)
							{
								Step.Longitude = (*LocationArray)[0]->AsNumber();
								Step.Latitude  = (*LocationArray)[1]->AsNumber();
							}

							(*ManeuverObj)->TryGetStringField(TEXT("instruction"), Step.Action);
						}
						

						Step.Distance = StepObj->GetNumberField(TEXT("distance"));
						Step.Duration = StepObj->GetNumberField(TEXT("duration"));


						

						Route->Steps.Add(Step);
						
					}
				}
			}
		}
	}

	// ✅ Return pointer via callback
	if (callback)
	{
		callback(static_cast<void*>(Route));
	}

}



void* GISNavigationFetcher::GetFallbackResource()
{
	return nullptr;
}
