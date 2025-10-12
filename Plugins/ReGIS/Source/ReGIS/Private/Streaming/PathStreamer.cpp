#include "Streaming/PathStreamer.h"


Journey::Journey(FRoute& InRouteData, int32 InJourneyID)
{
	Initialize(InRouteData, InJourneyID);
}

void Journey::Initialize(FRoute& InRouteData, int32 InJourneyID)
{
	RouteData = InRouteData;
	JourneyID = InJourneyID;
	DestinationCoordinate = RouteData.Geometry.Last(0) ;
}

FString Journey::GetCurrentInstruction()
{
	return RouteData.Steps[CurrentStepIndex].Action;
}

void Journey::TrimPathByLocation(GeoCoordinate CurrentCoordinateLocation)
{
	int32 ClosestStepIndex = FindClosestStepIndex(CurrentCoordinateLocation);
	int32 ClosestSubStepGeomtryIndex=FindClosestSubStepIndex(CurrentCoordinateLocation, ClosestStepIndex);
	if (ClosestStepIndex < 0 || ClosestStepIndex >= RouteData.Steps.Num())
		return;

	// this calculates where that step’s geometry starts in the flat array
	int32 GeoIndex = 0;
	for (int32 i = 0; i < ClosestStepIndex; ++i)
	{
		GeoIndex += RouteData.Steps[i].Geometry.Num();
	}
	GeoIndex+= ClosestSubStepGeomtryIndex;

	CurrentSubStepGeometryIndex = ClosestSubStepGeomtryIndex;
	CurrentGeometryIndex = GeoIndex;

}

TArray<GeoCoordinate> Journey::GetRemainingGeometry() const
{
	TArray<GeoCoordinate> Remaining;

	if (RouteData.Geometry.IsEmpty())
		return Remaining;

	for (int32 i = CurrentGeometryIndex; i < RouteData.Geometry.Num(); ++i)
	{
		Remaining.Add(RouteData.Geometry[i]);
	}

	return Remaining;
}

int32 Journey::FindClosestStepIndex(const GeoCoordinate& CurrentWorldLocation) const
{
	double MinDist = DBL_MAX;
	int32 Closest = 0;
	
	for (int32 i = 0; i < RouteData.Steps.Num(); ++i)
	{
		const auto& Step = RouteData.Steps[i];
		FVector StepWorld(Step.Longitude, Step.Latitude,0);
		FVector CurrentLocation(CurrentWorldLocation.Longitude,CurrentWorldLocation.Latitude,0);
		double D = FVector::Dist(CurrentLocation, StepWorld);
		if (D < MinDist)
		{
			MinDist = D;
			Closest = i;
		}
	}
	return Closest;
}

int32 Journey::FindClosestSubStepIndex(const GeoCoordinate& CurrentWorldLocation, int32 InCurrentGeometryIndex) const
{
	double MinDist = DBL_MAX;
	int32 Closest = 0;
	
	TArray<GeoCoordinate> SubGeometry = RouteData.Steps[InCurrentGeometryIndex].Geometry; 
	for (int32 i = 0; i < SubGeometry.Num(); ++i)
	{
		FVector StepWorld(SubGeometry[i].Longitude, SubGeometry[i].Latitude,0);
		FVector CurrentLocation(CurrentWorldLocation.Longitude,CurrentWorldLocation.Latitude,0);
		double D = FVector::Dist(CurrentLocation, StepWorld);
		if (D < MinDist)
		{
			MinDist = D;
			Closest = i;
		}
	}
	return Closest;
}

Journey* PathStreamer::CreateJourney(FRoute& RouteData,int32 InJourneyID )
{
	if (JourneyMap.Contains(InJourneyID))
	{
		return JourneyMap[InJourneyID].Get();
	}
	JourneyMap.Add(InJourneyID, MakeUnique<Journey>(RouteData,InJourneyID));
	return JourneyMap[InJourneyID].Get();
}

bool PathStreamer::RemoveJourney(int32 InJourneyID)
{
	if (JourneyMap.Contains(InJourneyID))
	{
		JourneyMap.Remove(InJourneyID);	
		return true;
	}
	return false;
}

void PathStreamer::ReCalculateJourney(FRoute& InRouteData, int32 InJourneyID)
{
	JourneyMap[InJourneyID]->Initialize(InRouteData, InJourneyID);
	JourneyMap[InJourneyID]->TrimPathByLocation(CurrentCoordinate);
}


Journey* PathStreamer::GetJourney(int32 JourneyID)
{
	return JourneyMap[JourneyID].Get();
}

void PathStreamer::UpdateCurrentCoordinate(GeoCoordinate UpdatedCoordinate)
{
	CurrentCoordinate = UpdatedCoordinate;
	for (auto& pair : JourneyMap)
	{
		pair.Value->TrimPathByLocation(UpdatedCoordinate);
		// Update UI broadcast for linked UI's or perhaps call that in parent!
	}
}




