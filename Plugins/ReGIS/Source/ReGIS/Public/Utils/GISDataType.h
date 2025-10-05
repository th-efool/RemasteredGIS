// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GISDataType.generated.h"


UCLASS()
class REGIS_API UGISDataType : public UObject
{
	GENERATED_BODY()
	
};


struct IGISCustomDatatypes{
};


struct FGISPoint : IGISCustomDatatypes
{
	double Latitude;
	double Longitude;
	float Altitude;
	int Zoom;
	FGISPoint()
		: Latitude(0.0f), Longitude(0.0f), Altitude(0.0f), Zoom(0)
	{}
	FGISPoint(double LatitudeDeg, double LongitudeDeg, float Altitude, int Zoom)
		: Latitude(LatitudeDeg), Longitude(LongitudeDeg),Altitude(Altitude),Zoom(Zoom)
	{}
				
};



USTRUCT(BlueprintType)
struct FGISStreamingConfig
{
	GENERATED_BODY();
	int32 TileSize = 256;
	UPROPERTY(EditAnywhere)
	int8 AtlasGridLength = 4;
	UPROPERTY(EditAnywhere)
	int8 CameraGridLength = 2;
};



USTRUCT(BlueprintType)
struct FInputTileData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile")
	int ZoomLevel = 14;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile")
	bool UseLatitudeLongitude = true;

	// Only editable when UseLatitudeLongitude == false
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile", meta=(EditCondition="!UseLatitudeLongitude"))
	int CenterX = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile", meta=(EditCondition="!UseLatitudeLongitude"))
	int CenterY = 100;

	// Only editable when UseLatitudeLongitude == true
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile", meta=(EditCondition="UseLatitudeLongitude"))
	double Latitude = 28.61;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile", meta=(EditCondition="UseLatitudeLongitude"))
	double Longitude = 77.23;
};

