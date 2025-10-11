// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
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



USTRUCT(BlueprintType)
struct FGISMeshAssets
{
	GENERATED_BODY();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetsRef")
	UStaticMesh* MeshAsset = nullptr;    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetsRef")    
	UMaterialInterface* MaterialAsset = nullptr;
};

USTRUCT(BlueprintType)
struct FGISMeshInstance
{
	GENERATED_BODY();
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial = nullptr;    
	UPROPERTY()
	UStaticMeshComponent* TileMesh = nullptr;

	void SetupSceneMesh()
	{
		TileMesh->SetWorldScale3D(FVector(10,10,1));
		TileMesh->SetGenerateOverlapEvents(true);
		TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		TileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		TileMesh->SetCollisionObjectType(ECC_WorldStatic);

		TileMesh->SetNotifyRigidBodyCollision(true);
		TileMesh->SetCollisionResponseToAllChannels(ECR_Block);
		TileMesh->bSelectable = true;

		// Let it receive input clicks
		TileMesh->SetEnableGravity(false);
		TileMesh->bEditableWhenInherited = true;
		
	}
	void Refresh(FGISMeshAssets Assets)
	{
		if (Assets.MaterialAsset != nullptr)  
		{  
			DynamicMaterial = UMaterialInstanceDynamic::Create(Assets.MaterialAsset, TileMesh);  
		}  
		if (Assets.MeshAsset != nullptr)  
		{  
			TileMesh->SetStaticMesh(Assets.MeshAsset);  
		}
	};
	void SetMaterialFromStreamingTexture(UTexture2D* InStreamingTexture)
	{
		DynamicMaterial->SetTextureParameterValue("BaseColor",InStreamingTexture);
		TileMesh->SetMaterial(0, DynamicMaterial);
	};
};




USTRUCT(BlueprintType)
struct FParamsCanvasClickedDelegate
{
	GENERATED_BODY()
	UPROPERTY()
	double Longitude=0.0;
	UPROPERTY()
	double Latitude=0.0;
	UPROPERTY()
	FVector LocalHitCoord;
	
	FParamsCanvasClickedDelegate()
		: Longitude(0.0)
		, Latitude(0.0)
		, LocalHitCoord(FVector::ZeroVector) // same as FVector(0,0,0)
	{}
};