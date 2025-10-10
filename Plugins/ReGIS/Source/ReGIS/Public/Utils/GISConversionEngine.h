// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <cmath>
#include <utility>
#include "DBMS/FGISIdentifier.h"

#include "Utils/GISDataType.h"

/**
 * 
 */
class REGIS_API GISConversionEngine
{
public:
	GISConversionEngine();

		static FGISTileID  LatLonToTile(FGISPoint Point, int Zoom);
		static FGISTileID  LatLonToTile(double Latitude, double Longitude, int Zoom);
		// Convert Lat/Lon to Tile X/Y at given Zoom
		static std::pair<int, int> LatLonToTile(FGISPoint Point);

	// Convert Tile X/Y at Zoom back to Lat/Lon (tile top-left corner)
		static std::pair<double, double> TileToLatLon(FGISTileID TileID, FVector2D Offset);

		static FVector2D CalculateOffsetInUnitTileLength(FGISTileID TileID, double Latitude, double Longitude);
	
	~GISConversionEngine();
};
