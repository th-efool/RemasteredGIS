// Fill out your copyright notice in the Description page of Project Settings.


#include "GISStaticTileViewportActor.h"
#include "DBMS/DataManager.h"
#include "Utils/GISErrorHandler.h"

AGISStaticTileViewportActor::AGISStaticTileViewportActor()
{
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));  
	TileMesh->SetupAttachment(RootSceneComponent);  
	TileMesh->SetWorldScale3D(FVector(5,5,1));
	FetchIndex=0;
}



void AGISStaticTileViewportActor::BeginPlay()
{
	Super::BeginPlay();
	InitStaticStreaming();

	FTimerHandle TempHandle;
	GetWorldTimerManager().SetTimer(TempHandle, [this]()
	{
		UDataManager* DataManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDataManager>() : nullptr;
		if (!DataManager)
		{
			UE_LOG(LogTemp, Warning, TEXT("DataManager not ready yet!"));
			return;
		}

		if (!StaticStreamer)
		{
			UE_LOG(LogTemp, Warning, TEXT("StaticStreamer not ready!"));
			return;
		}
		FetchVisibleTiles();
	

	}, 1.0f, false);


}


void AGISStaticTileViewportActor::Tick(float DeltaTime)  
{  
	Super::Tick(DeltaTime);  
	TestCameraMovement();
	
	
  
}

void AGISStaticTileViewportActor::FetchVisibleTiles()
{
	FetchIndex++;
	unsigned int ThisFetchIndex = FetchIndex;
	if (!StaticStreamer){return;}
	int TopLeftCornerX = CenterTileID.X-0.5*(StaticStreamer->AtlasTileCountX);
	int TopLeftCornerY = CenterTileID.Y-0.5*(StaticStreamer->AtlasTileCountY);
	int Zoom = CenterTileID.ZoomLevel;
	if (TopLeftCornerX<0 || TopLeftCornerY<0 || Zoom<0)
	{
		return;
	}
	TWeakObjectPtr<AGISStaticTileViewportActor> WeakThis(this);

	

	UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>();

	
	StaticStreamer->ReInitVisibleTiles(); // Converting All Tiles TO WHITE TILES 
	for (int i=0; i< StaticStreamer->AtlasTileCountX; i++)
	{
		for (int j=0; j< StaticStreamer->AtlasTileCountY; j++)
		{
			int TileIndex = j*(StaticStreamer->AtlasTileCountY)+i;
			FGISTileID TileID = FGISTileID(Zoom, TopLeftCornerX+i, TopLeftCornerY+j);
			TFunction<void(UTexture2D*)> Callback =
			[WeakThis,ThisFetchIndex,TileIndex ](UTexture2D* InTexture)
			{
				if (WeakThis.IsValid())
					{
						WeakThis->HandleTexture(InTexture,ThisFetchIndex,TileIndex);
					}
			};
			DataManager->GetStaticTile(TileID, Callback );
		}
	}

}

void AGISStaticTileViewportActor::HandleTexture(UTexture2D* Texture, unsigned int fetchIndex, int TileIndex) const
{
	
	if (FetchIndex == fetchIndex)
	{		UE_LOG(LogTemp, Display, TEXT("SUCCESSFUL  FetchIndex: %d , TileIndex: %d"), fetchIndex, TileIndex)
		StaticStreamer->SetVisibleTileIndexed(Texture, TileIndex);
	} else
	{
		UE_LOG(LogTemp, Display, TEXT("FAILED  FetchIndex: %d , TileIndex: %d"), fetchIndex, TileIndex)
	
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

	// Base sinusoidal motion (slower, smoother)
	float BaseX = FMath::Sin(TimeElapsed * 0.2f * 2 * PI); // slower than 0.5
	float BaseY = FMath::Sin(TimeElapsed * 0.15f * 2 * PI + PI / 2);

	// Gentle noise for natural variation
	float NoiseX = FMath::PerlinNoise1D(TimeElapsed * 0.3f) * 0.2f; // subtle, slow
	float NoiseY = FMath::PerlinNoise1D((TimeElapsed + 1000.f) * 0.3f) * 0.2f;

	// Weighted sum
	float OffsetX = FMath::Clamp(BaseX * 0.8f + NoiseX, -1.f, 1.f);
	float OffsetY = FMath::Clamp(BaseY * 0.8f + NoiseY, -1.f, 1.f);

	// Optional: smooth over time using Lerp
	static float SmoothedX = 0.f;
	static float SmoothedY = 0.f;
	float SmoothSpeed = 2.f; // higher = faster response
	SmoothedX = FMath::FInterpTo(SmoothedX, OffsetX, GetWorld()->GetDeltaSeconds(), SmoothSpeed);
	SmoothedY = FMath::FInterpTo(SmoothedY, OffsetY, GetWorld()->GetDeltaSeconds(), SmoothSpeed);

	StaticStreamer->SetCameraOffset(SmoothedX, SmoothedY);
}

