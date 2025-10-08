// Fill out your copyright notice in the Description page of Project Settings.


#include "GISComponents/GISStaticTileRendererComponent.h"

#include "DBMS/DataManager.h"
#include "Utils/GISConversionEngine.h"

// Sets default values for this component's properties
UGISStaticTileRendererComponent::UGISStaticTileRendererComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	FetchIndex=0;
}


FGISTileID UGISStaticTileRendererComponent::GetLogicalCenterTileID()
{return CenterTileID;}

FGISTileID UGISStaticTileRendererComponent::GetVisualCenterTileID()
{
	if (VisibleTilesID.Num()>0)
	{
		return VisibleTilesID[GetCenterTileIndex()];
	}
	return CenterTileID;
}

int UGISStaticTileRendererComponent::GetCenterTileIndex() const
{
	int mid = (StreamingConfig.AtlasGridLength - 1) / 2;          // 0-based row/col
	int centerIndex = mid * StreamingConfig.AtlasGridLength + mid + 1; // 1-based sequential index
	centerIndex-=1; //Counting Starts From Zero
	return centerIndex;
}



void UGISStaticTileRendererComponent::UpdateLocation(FGISTileID newCentertileID)
{
	CenterTileID = newCentertileID;
	FetchVisibleTiles();
}

void UGISStaticTileRendererComponent::UpdateLocation(FGISPoint newCenterLongLat)
{
	auto [x, y] = GISConversionEngine::LatLonToTile(newCenterLongLat);
	CenterTileID = FGISTileID(newCenterLongLat.Zoom, x, y);
	FetchVisibleTiles();
}

void UGISStaticTileRendererComponent::Pan(float& X, float& Y)
{
	if (X>0.8 || Y>0.8 || X<-0.8 || Y<-0.8)
	{
		FGISTileID NewCenter = CenterTileID;
		if (X>0.8)
		{
			NewCenter.X++;
			X=X-1;
		}
		if (Y>0.8)
		{
			NewCenter.Y++;
			Y=Y-1;
		}
		if (X<-0.8)
		{
			NewCenter.X--;
			X=X+1;
		}
		if (Y<-0.8)
		{
			NewCenter.Y--;
			Y=Y+1;
		}
		UpdateLocation(NewCenter);
	}
	StaticStreamer->SetCameraOffset(X, Y);
}

void UGISStaticTileRendererComponent::Zoom(int Delta)
{
	CenterTileID.ZoomLevel += Delta;
	UpdateLocation(CenterTileID);
}

void UGISStaticTileRendererComponent::FetchVisibleTiles()
{
	UDataManager* DataManager= GetWorld()->GetGameInstance()->GetSubsystem<UDataManager>();
	if (DataManager==nullptr){return;}
	
	FetchIndex++;
	unsigned int ThisFetchIndex = FetchIndex;
	if (!StaticStreamer){return;}
	int TopLeftCornerX = CenterTileID.X-0.5*(StreamingConfig.AtlasGridLength);
	int TopLeftCornerY = CenterTileID.Y-0.5*(StreamingConfig.AtlasGridLength);
	int Zoom = CenterTileID.ZoomLevel;
	if (TopLeftCornerX<0 || TopLeftCornerY<0 || Zoom<0){return;}
	TWeakObjectPtr<UGISStaticTileRendererComponent> WeakThis(this);

	VisibleTilesID.Empty();
	StaticStreamer->ReInitVisibleTiles(); // Converting All Tiles TO WHITE TILES 
	for (int i=0; i< StreamingConfig.AtlasGridLength; i++)
	{
		for (int j=0; j< StreamingConfig.AtlasGridLength; j++)
		{
			int TileIndex = j*(StreamingConfig.AtlasGridLength)+i;
			FGISTileID TileID = FGISTileID(Zoom, TopLeftCornerX+i, TopLeftCornerY+j);
			VisibleTilesID.Add(TileID);
			TFunction<void(UTexture2D*)> Callback =
			[WeakThis,ThisFetchIndex,TileIndex ](UTexture2D* InTexture)
			{
				if (WeakThis.IsValid())
				{WeakThis->HandleTexture(InTexture,ThisFetchIndex,TileIndex);}
			};
			DataManager->GetStaticTile(TileID, Callback );
		}
	}
}

void UGISStaticTileRendererComponent::HandleTexture(UTexture2D* Texture, unsigned int fetchIndex, int TileIndex) const
{
	if (FetchIndex != fetchIndex){return;}
	StaticStreamer->SetVisibleTileIndexed(Texture, TileIndex);
}

/*void UGISStaticTileRendererComponent::OnConstruction(const FTransform& Transform)
{
	InitStaticStreaming();
}*/

// Called when the game starts
void UGISStaticTileRendererComponent::BeginPlay()
{
	Super::BeginPlay();

	InitStaticStreaming();
	
	FTimerHandle TempHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TempHandle,
		[this](){FetchVisibleTiles();},
		1.0f, false
		);
		
}


// Called every frame
void UGISStaticTileRendererComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TestCameraMovement();

	// ...
}

void UGISStaticTileRendererComponent::RefreshInputConfig(FGISStreamingConfig InStreamingConfig, FInputTileData InConfigData)
{
	StreamingConfig=InStreamingConfig;
	TileConfigData=InConfigData;
		
	if (TileConfigData.UseLatitudeLongitude)
	{
		FGISPoint Point= FGISPoint(TileConfigData.Latitude,TileConfigData.Longitude,0, TileConfigData.ZoomLevel);
		std::pair<int, int> TileID = GISConversionEngine::LatLonToTile(Point);	
		CenterTileID=FGISTileID(TileConfigData.ZoomLevel, TileID.first, TileID.second);

	} else
	{
		CenterTileID=FGISTileID(TileConfigData.ZoomLevel, TileConfigData.CenterX, TileConfigData.CenterY);
	}
	
}

void UGISStaticTileRendererComponent::RefreshInternalConfiguration()
{
	Super::RefreshInternalConfiguration();
}

void UGISStaticTileRendererComponent::InitStaticStreaming()
{
	StaticStreamer = new StaticStreaming(
		StreamingConfig.CameraGridLength,StreamingConfig.CameraGridLength,
		StreamingConfig.AtlasGridLength, StreamingConfig.AtlasGridLength,
		StreamingConfig.TileSize, StreamingConfig.TileSize
		);
	
	
	StaticStreamer->UpdateAtlas();
	StaticStreamer->UpdateStreaming();
}


void UGISStaticTileRendererComponent::TestCameraMovement()
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	float Time = GetWorld()->GetTimeSeconds();

	// --- Base motion: slower sinusoidal oscillation ---
	constexpr float FrequencyX = 0.08f; // ~1 cycle every ~12.5s
	constexpr float FrequencyY = 0.06f; // ~1 cycle every ~16.7s
	float BaseX = FMath::Sin(Time * FrequencyX * 2.f * PI);
	float BaseY = FMath::Sin(Time * FrequencyY * 2.f * PI + PI / 2.f);

	// --- Gentle Perlin noise for organic variation ---
	constexpr float NoiseFreq = 0.05f; // slower variation
	constexpr float NoiseAmplitude = 0.15f; // subtle
	float NoiseX = FMath::PerlinNoise1D(Time * NoiseFreq) * NoiseAmplitude;
	float NoiseY = FMath::PerlinNoise1D((Time + 1000.f) * NoiseFreq) * NoiseAmplitude;

	// --- Weighted combination ---
	float TargetX = FMath::Clamp(BaseX * 0.7f + NoiseX, -1.f, 1.f);
	float TargetY = FMath::Clamp(BaseY * 0.7f + NoiseY, -1.f, 1.f);

	// --- Smooth over time (low-pass) ---
	static float SmoothedX = 0.f;
	static float SmoothedY = 0.f;
	constexpr float SmoothInterpSpeed = 0.7f; // slower = softer
	SmoothedX = FMath::FInterpTo(SmoothedX, TargetX, DeltaTime, SmoothInterpSpeed);
	SmoothedY = FMath::FInterpTo(SmoothedY, TargetY, DeltaTime, SmoothInterpSpeed);

	// --- Apply camera offset ---
	// REPLACE WITH AN METHOD THAT USE GISSTATICTILERENDERERCOMPONENT
	StaticStreamer->SetCameraOffset(SmoothedX, SmoothedY);
}

