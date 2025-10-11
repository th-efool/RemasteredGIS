#pragma once
#include "Utils/GISDataType.h"

UENUM(BlueprintType)
enum class ENavigationProfile : uint8
{
	Driving         UMETA(DisplayName = "Driving"),
	Walking         UMETA(DisplayName = "Walking"),
	Cycling         UMETA(DisplayName = "Cycling"),
	DrivingTraffic  UMETA(DisplayName = "Driving with Traffic")
};
struct ParamsNavigationFetcher : public IGISCustomDatatypes
{
	double StartLongitude;
	double StartLatitude;
	
	double EndLongitude;
	double EndLatitude;

	ENavigationProfile ProfileType = ENavigationProfile::Driving;

	bool bAlternatives = true;
	bool bSteps = true;
	FString Overview = TEXT("full");
	FString Language = TEXT("en");


	// Default constructor
	ParamsNavigationFetcher()
		: StartLongitude(0.0), StartLatitude(0.0),
		  EndLongitude(0.0), EndLatitude(0.0),
		  ProfileType(ENavigationProfile::Driving),
		  bAlternatives(true), bSteps(true),
		  Overview(TEXT("full")), Language(TEXT("en"))
	{}

	// Constructor with start/end doubles
	ParamsNavigationFetcher(double InStartLongitude, double InStartLatitude,
							double InEndLongitude, double InEndLatitude)
		: StartLongitude(InStartLongitude), StartLatitude(InStartLatitude),
		  EndLongitude(InEndLongitude), EndLatitude(InEndLatitude)
	{}

	// Constructor with start/end + profile
	ParamsNavigationFetcher(double InStartLongitude, double InStartLatitude,
							double InEndLongitude, double InEndLatitude,
							ENavigationProfile InProfile)
		: StartLongitude(InStartLongitude), StartLatitude(InStartLatitude),
		  EndLongitude(InEndLongitude), EndLatitude(InEndLatitude),
		  ProfileType(InProfile)
	{}

	// Full constructor with all options
	ParamsNavigationFetcher(double InStartLongitude, double InStartLatitude,
							double InEndLongitude, double InEndLatitude,
							ENavigationProfile InProfile,
							bool bInAlternatives, bool bInSteps,
							const FString& InOverview, const FString& InLanguage)
		: StartLongitude(InStartLongitude), StartLatitude(InStartLatitude),
		  EndLongitude(InEndLongitude), EndLatitude(InEndLatitude),
		  ProfileType(InProfile),
		  bAlternatives(bInAlternatives), bSteps(bInSteps),
		  Overview(InOverview), Language(InLanguage)
	{}

};

struct GeoCoordinate
{
	double Longitude = 0.0;
	double Latitude = 0.0;
};

struct FRouteStep
{
	double Longitude = 0.0;
	double Latitude = 0.0;
	FString Action;
	double Distance = 0.0;
	double Duration = 0.0;
	TArray<GeoCoordinate> Geometry;
};

struct FRoute
{
	TArray<GeoCoordinate> Geometry;
	double Distance = 0.0;
	double Duration = 0.0;
	TArray<FRouteStep> Steps;
};

