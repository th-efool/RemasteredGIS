// Fill out your copyright notice in the Description page of Project Settings.


#include "GISStaticTileViewportActor.h"

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
	InCenterTile=FGISTileID(ZoomLevel, CenterX, CenterY);
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