#pragma once

#include "CoreMinimal.h"
#include "Utils/GISNavigationDatatype.h"      // for FRoute, GeoCoordinate

class Journey
{
public:
	FRoute RouteData;

	FString GetCurrentInstruction();
	void TrimPathByLocation(GeoCoordinate CurrentCoordinateLocation);
	TArray<GeoCoordinate> GetRemainingGeometry() const;

public:
	Journey(FRoute& InRouteData, int32 InJourneyID);
	void Initialize(FRoute& InRouteData, int32 InJourneyID);
	GeoCoordinate DestinationCoordinate;
	int32 JourneyID = -1;
private:
	int32 CurrentStepIndex = 0;
	int32 CurrentSubStepGeometryIndex = 0;
	int32 CurrentGeometryIndex = 0;
	int32 FindClosestStepIndex(const GeoCoordinate& CurrentWorldLocation) const;
	int32 FindClosestSubStepIndex(const GeoCoordinate& CurrentWorldLocation, int32 CurrentGeometryIndex) const;
};



class PathStreamer
{
public:

	// Owns all journeys
	TMap<int32, TUniquePtr<Journey>> JourneyMap;

	
	// Create and store new journey
	Journey* CreateJourney(FRoute& RouteData,int32 InJourneyID );

	// Remove journey by ID
	bool RemoveJourney(int32 JourneyID);

	void ReCalculateJourney(FRoute& RouteData,int32 InJourneyID); 
	
	// Access journey
	Journey* GetJourney(int32 JourneyID);

	void UpdateCurrentCoordinate(GeoCoordinate UpdatedCoordinate);
private:
	
	GeoCoordinate CurrentCoordinate;
public:
	PathStreamer() = default;
	~PathStreamer()= default;
};
