// Fill out your copyright notice in the Description page of Project Settings.


#include "GISStaticTileViewportActor.h"
#include "DBMS/DataManager.h"
#include "Utils/GISErrorHandler.h"
#include "Utils/GISConversionEngine.h"

AGISStaticTileViewportActor::AGISStaticTileViewportActor()
{
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));  
	TileMesh->SetupAttachment(RootSceneComponent);  
	TileMesh->SetWorldScale3D(FVector(5,5,1));
	FetchIndex=0;
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

int AGISStaticTileViewportActor::GetCenterTileIndex() const
{int mid = (InStreamingConfig.GridLengthX - 1) / 2;          // 0-based row/col
	int centerIndex = mid * InStreamingConfig.GridLengthX + mid + 1; // 1-based sequential index
	centerIndex-=1; //Counting Starts From Zero
	return centerIndex;
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
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		// Give this actor input focus
		EnableInput(PC);

		// Optional: show mouse cursor for testing
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
	}

	if (TileMesh){
	TileMesh->OnClicked.AddDynamic(this, &AGISStaticTileViewportActor::HandleOnClicked);
	}
	

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
	int TopLeftCornerX = CenterTileID.X-0.5*(InStreamingConfig.GridLengthX);
	int TopLeftCornerY = CenterTileID.Y-0.5*(InStreamingConfig.GridLengthY);
	int Zoom = CenterTileID.ZoomLevel;
	if (TopLeftCornerX<0 || TopLeftCornerY<0 || Zoom<0)
	{
		return;
	}
	TWeakObjectPtr<AGISStaticTileViewportActor> WeakThis(this);

	

	UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>();

	VisibleTilesID.Empty();
	StaticStreamer->ReInitVisibleTiles(); // Converting All Tiles TO WHITE TILES 
	for (int i=0; i< InStreamingConfig.GridLengthX; i++)
	{
		for (int j=0; j< InStreamingConfig.GridLengthY; j++)
		{
			int TileIndex = j*(InStreamingConfig.GridLengthY)+i;
			FGISTileID TileID = FGISTileID(Zoom, TopLeftCornerX+i, TopLeftCornerY+j);
			VisibleTilesID.Add(TileID);
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
	if (InputConfigData.UseLatitudeLongitude)
	{
		FGISPoint Point= FGISPoint(InputConfigData.Latitude,InputConfigData.Longitude,0, InputConfigData.ZoomLevel);
		std::pair<int, int> TileID = GISConversionEngine::LatLonToTile(Point);	
		CenterTileID=FGISTileID(InputConfigData.ZoomLevel, TileID.first, TileID.second);

	} else
	{
		CenterTileID=FGISTileID(InputConfigData.ZoomLevel, InputConfigData.CenterX, InputConfigData.CenterY);
	}
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
	StaticStreamer->SetCameraOffset(SmoothedX, SmoothedY);
}

void AGISStaticTileViewportActor::HandleOnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	// Get hit under cursor from player controller
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	FHitResult Hit;
	if (PC->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
	{
		if (Hit.GetActor() == this)
		{
			// Convert to local coords
			FVector LocalPoint = TileMesh->GetComponentTransform().InverseTransformPosition(Hit.ImpactPoint);
			FVector2D Local2D(LocalPoint.X, LocalPoint.Y);

			// Fire delegate
			OnCanvasClicked.Broadcast(Local2D);

			ConvertLocalPointToGISPoint(Local2D);
		}
	}
}

FGISPoint AGISStaticTileViewportActor::ConvertLocalPointToGISPoint(FVector2D LocalCoord) const
{
	// 1) Invert Y to match coordinate convention
	double inputY = -LocalCoord.Y;

	// 2) Scale input coordinates (/100 step)
	double scaledX = LocalCoord.X / 50.0;
	double scaledY = inputY / 50.0;

	// 3) Unit lengths from center (after scaling by grid)
	double unitLenFromCenterX = scaledX * (0.5f*InStreamingConfig.CameraGridLengthX);
	double unitLenFromCenterY = scaledY * (0.5*InStreamingConfig.CameraGridLengthY);

	// 4) [Reserved] pseudo center tile distance (relative to center tile top-left)
	double CenterTileTLX = (InStreamingConfig.GridLengthX%2 ==1) ? -0.5f : -1.f; 
	double CenterTileTLY = (InStreamingConfig.GridLengthY%2 ==1) ? 0.5f : 1.f;

	// 5) Camera offsets (raw + localized)
	FVector2D cameraOffsetRaw = StaticStreamer->GetCameraOffset();
	float MaxPossibleTileOffsetX= (InStreamingConfig.GridLengthX-InStreamingConfig.CameraGridLengthX)/2.f;
	float MaxPossibleTileOffsetY= (InStreamingConfig.GridLengthX-InStreamingConfig.CameraGridLengthY)/2.f;
	double localizedCamOffsetX = -(cameraOffsetRaw.X ) * MaxPossibleTileOffsetX;
	double localizedCamOffsetY = (cameraOffsetRaw.Y ) * MaxPossibleTileOffsetY;

	// 6) Final center tile distances
	double centerTileDistanceX = unitLenFromCenterX-(CenterTileTLX+localizedCamOffsetX);
	double centerTileDistanceY = unitLenFromCenterY-(CenterTileTLY+localizedCamOffsetY);
	
	// 7) Build minimal debug message
	FString TraceMsg;
	TraceMsg += FString::Printf(TEXT("CANVAS LOCAL HIT TRACE\n"));
	TraceMsg += FString::Printf(TEXT("1) DistanceFromCenter (inTileUnit): X=%.4f, Y=%.4f\n"), unitLenFromCenterX, unitLenFromCenterY);
	TraceMsg += FString::Printf(TEXT("2) CENTER TILE TOP LEFT:        X=%.4f, Y=%.4f\n"),
								CenterTileTLX, CenterTileTLY);
	TraceMsg += FString::Printf(TEXT("3) CAMERA OFFSET RAW:              X=%.4f, Y=%.4f\n"), cameraOffsetRaw.X, cameraOffsetRaw.Y);
	TraceMsg += FString::Printf(TEXT("   LOCALIZED CAM OFFSET:           X=%.4f, Y=%.4f\n"),
								localizedCamOffsetX, localizedCamOffsetY);
	TraceMsg += FString::Printf(TEXT("4) FINAL CenterTileDistance:       X=%.4f, Y=%.4f\n"), centerTileDistanceX, centerTileDistanceY);

	// 8) Single on-screen print
	GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green, TraceMsg);

	std::pair<double, double> LatLong = GISConversionEngine::TileToLatLon(VisibleTilesID[GetCenterTileIndex()], FVector2D(centerTileDistanceX,-centerTileDistanceY));

	FString Msg = FString::Printf(TEXT("Lat: %.6f, Lon: %.6f"), LatLong.first, LatLong.second);

	// Print to screen for 5 seconds in green
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, Msg);

	// 9) Return the computed GIS point
	return FGISPoint(LatLong.first, LatLong.second, 0, CenterTileID.ZoomLevel);

}

