// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/GISConversionEngine.h"

GISConversionEngine::GISConversionEngine()
{
}

std::pair<int, int> GISConversionEngine::LatLonToTile(FGISPoint Point)
{
	double LatitudeDeg = Point.Latitude;
	double LongitudeDeg = Point.Longitude;
	int Zoom = Point.Zoom;
	
	LatitudeDeg = FMath::Clamp(LatitudeDeg, -85.05112878, 85.05112878); // Mercator limit

	double LatRad = FMath::DegreesToRadians(LatitudeDeg);
	double N = std::pow(2.0, Zoom);

	int TileX = static_cast<int>( (LongitudeDeg + 180.0) / 360.0 * N );
	int TileY = static_cast<int>( (1.0 - std::log(std::tan(LatRad) + 1.0 / std::cos(LatRad)) / PI) / 2.0 * N );

	return { TileX, TileY };
}

std::pair<double, double> GISConversionEngine::TileToLatLon(FGISTileID TileID, FVector2D Offset)
{
	int TileX = TileID.X;
	int TileY = TileID.Y;
	int Zoom = TileID.ZoomLevel;

	float X = TileX + Offset.X;
	float Y = TileY + Offset.Y;
	
	double N = std::pow(2.0, Zoom);

	double LonDeg = X / N * 360.0 - 180.0;
	double LatRad = std::atan(std::sinh(PI * (1.0 - 2.0 * Y / N)));
	double LatDeg = FMath::RadiansToDegrees(LatRad);

	return { LatDeg, LonDeg };
}



GISConversionEngine::~GISConversionEngine()
{
}
