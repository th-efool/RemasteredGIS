// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/GISConversionEngine.h"

GISConversionEngine::GISConversionEngine()
{
}

FGISTileID GISConversionEngine::LatLonToTile(double Latitude, double Longitude, int Zoom)
{
	Latitude = FMath::Clamp(Latitude, -85.05112878, 85.05112878); // Mercator limit

	double LatRad = FMath::DegreesToRadians(Latitude);
	double N = std::pow(2.0, Zoom);

	int TileX = static_cast<int>( (Longitude + 180.0) / 360.0 * N );
	int TileY = static_cast<int>( (1.0 - std::log(std::tan(LatRad) + 1.0 / std::cos(LatRad)) / PI) / 2.0 * N );

	return FGISTileID(TileX, TileY,Zoom);
}


FGISTileID GISConversionEngine::LatLonToTile(FGISPoint Point, int InZoom)
{
	double LatitudeDeg = Point.Latitude;
	double LongitudeDeg = Point.Longitude;
	int Zoom = InZoom;
	return 	LatLonToTile(LatitudeDeg, LongitudeDeg, Zoom);
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

FVector2D GISConversionEngine::CalculateOffsetInUnitTileLength(FGISTileID TileID, double Latitude, double Longitude)
{
	int TileX = TileID.X;
	int TileY = TileID.Y;
	int Zoom = TileID.ZoomLevel;

	double N = std::pow(2.0, Zoom);

	// 1) Convert lat/lon to tile-space (X,Y)
	double LonDeg = Longitude;
	double LatRad = FMath::DegreesToRadians(Latitude);

	double X = (LonDeg + 180.0) / 360.0 * N;
	double Y = (1.0 - std::log(std::tan(LatRad) + 1.0 / std::cos(LatRad)) / PI) / 2.0 * N;

	// 2) Compute offset relative to the given tile
	double OffsetX = X - TileX;
	double OffsetY = Y - TileY;

	// 3) Return as FVector2D (matches TileToLatLon’s Offset input convention)
	return FVector2D(OffsetX, OffsetY);
}



GISConversionEngine::~GISConversionEngine()
{
}
 