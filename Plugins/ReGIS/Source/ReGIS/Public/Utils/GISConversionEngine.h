// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <cmath>
#include <utility>

#include "Utils/GISDataType.h"

/**
 * 
 */
class REGIS_API GISConversionEngine
{
public:
	GISConversionEngine();
		// Convert Lat/Lon to Tile X/Y at given Zoom
		static std::pair<int, int> LatLonToTile(FGISPoint Point);

	// Convert Tile X/Y at Zoom back to Lat/Lon (tile top-left corner)
		static std::pair<double, double> TileToLatLon(FGISTileID TileID, FVector2D Offset);
	
	
	~GISConversionEngine();
};
