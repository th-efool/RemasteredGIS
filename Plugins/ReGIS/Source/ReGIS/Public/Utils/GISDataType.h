// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DBMS/FGISIdentifier.h"
#include "UObject/NoExportTypes.h"
#include "GISDataType.generated.h"

/**
 * 
 */
UCLASS()
class REGIS_API UGISDataType : public UObject
{
	GENERATED_BODY()
	
};


struct ICustomParams{
};


struct FGISPoint
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
	UPROPERTY(EditAnywhere)
	int32 TileSizeX = 256;
	UPROPERTY(EditAnywhere)
	int32 TileSizeY = 256;
	UPROPERTY(EditAnywhere)
	int8 GridLengthX = 4;
	UPROPERTY(EditAnywhere)
	int8 GridLengthY = 4;
	UPROPERTY(EditAnywhere)
	int8 CameraGridLengthX = 2;
	UPROPERTY(EditAnywhere)
	int8 CameraGridLengthY = 2;
};

