// Fill out your copyright notice in the Description page of Project Settings.


#include "GISStaticTileViewportActor.h"
#include "DBMS/DataManager.h"
#include "Utils/GISErrorHandler.h"

AGISStaticTileViewportActor::AGISStaticTileViewportActor()
{
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));  
	TileMesh->SetupAttachment(RootSceneComponent);  
	TileMesh->SetWorldScale3D(FVector(5,5,1));  
}



void AGISStaticTileViewportActor::BeginPlay()
{
	Super::BeginPlay();
	InitStaticStreaming();

}


void AGISStaticTileViewportActor::Tick(float DeltaTime)  
{  
	Super::Tick(DeltaTime);  

	TestCameraMovement();
	
	
  
}

void AGISStaticTileViewportActor::FetchVisibleTiles()
{
	if (!StaticStreamer){return;}
	int TopLeftCornerX = CenterTileID.X-0.5*(StaticStreamer->AtlasTileCountX);
	int TopLeftCornerY = CenterTileID.Y-0.5*(StaticStreamer->AtlasTileCountY);
	int Zoom = CenterTileID.ZoomLevel;
	if (TopLeftCornerX<0 || TopLeftCornerY<0 || Zoom<0)
	{
		return;
	}
	UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>();
	for (int i=0; i< StaticStreamer->AtlasTileCountX; i++)
	{
		for (int j=0; j< StaticStreamer->AtlasTileCountY; j++)
		{
			FGISTileID TileID = FGISTileID(Zoom, TopLeftCornerX+i, TopLeftCornerY+j);
			
			DataManager->GetStaticTile(TileID, );
		}
	}

}

void AGISStaticTileViewportActor::InitStaticStreaming()
{
	StaticStreamer = new StaticStreaming(
		InStreamingConfig.CameraGridLengthX,InStreamingConfig.CameraGridLengthY,
		InStreamingConfig.GridLengthX, InStreamingConfig.GridLengthY,
		InStreamingConfig.TileSizeX, InStreamingConfig.TileSizeY
		);
	
	GIS_HANDLE_IF (DynamicMaterial != nullptr)
	{
		DynamicMaterial->SetTextureParameterValue("BaseColor",StaticStreamer->GetStreamingTexture());
		TileMesh->SetMaterial(0, DynamicMaterial);
	}
	StaticStreamer->UpdateAtlas();
	StaticStreamer->UpdateStreaming();
	
}

void AGISStaticTileViewportActor::RefreshConfig()
{
	Super::RefreshConfig();
	CenterTileID=FGISTileID(ZoomLevel, CenterX, CenterY);
	GIS_HANDLE_IF (TileBaseMaterialAsset)  
	{  
		DynamicMaterial = UMaterialInstanceDynamic::Create(TileBaseMaterialAsset, TileMesh);  
	}  
	GIS_HANDLE_IF (TileBaseMeshAsset)  
	{  
		TileMesh->SetStaticMesh(TileBaseMeshAsset);  
	}
	
}



void AGISStaticTileViewportActor::TestCameraMovement()
{
	float TimeElapsed = GetWorld()->GetTimeSeconds(); // continuous time
	
	float BaseX = FMath::Sin(TimeElapsed * 0.5f * 2 * PI);
	float BaseY = FMath::Sin(TimeElapsed * 0.7f * 2 * PI + PI/2);

	float NoiseX = FMath::PerlinNoise1D(TimeElapsed * 0.8f) * 2.f - 1.f;
	float NoiseY = FMath::PerlinNoise1D((TimeElapsed + 1000.f) * 0.8f) * 2.f - 1.f;

	// Combine with weighted sum and scale to fit [-1,1]
	float OffsetX = FMath::Clamp(BaseX * 0.9f + NoiseX * 0.1f, -1.f, 1.f);
	float OffsetY = FMath::Clamp(BaseY * 0.9f + NoiseY * 0.1f, -1.f, 1.f);
	
	// Apply to streaming system
	StaticStreamer->SetCameraOffset(OffsetX, OffsetY);  
}
