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
	float Latitude;
	float Longitude;
	float Altitude;
	FGISPoint()
		: Latitude(0.0f), Longitude(0.0f), Altitude(0.0f)
	{
	}
};



