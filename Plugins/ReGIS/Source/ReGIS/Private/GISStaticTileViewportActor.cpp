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
		/*FetchVisibleTiles();*/
	

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
	/*
	TestCameraMovement();
	*/
	
	
  
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

	
	StaticStreamer->ReInitVisibleTiles(); // Converting All Tiles TO WHITE TILES 
	for (int i=0; i< InStreamingConfig.GridLengthX; i++)
	{
		for (int j=0; j< InStreamingConfig.GridLengthY; j++)
		{
			int TileIndex = j*(InStreamingConfig.GridLengthY)+i;
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
	// Scaling down so we can play in local units more properly
	double X = LocalCoord.X/100.f;
	double Y = LocalCoord.Y/100.f;
	// The Plane is 100x100 units w/ UpperLeftCorner(-50,-50), UpperRightCorner(50,-50), DownRightCorner(50,50), Center(0,0)
	// So Inverting Y value for consistency in calculations
	Y = -Y;
	
	double UnitLengthFromCenterPointX=X*InStreamingConfig.CameraGridLengthX;
	double UnitLengthFromCenterPointY=Y*InStreamingConfig.CameraGridLengthY;

	// Unit Length from Top Left Corner of the CenterTile
	double PseudoCenterTileDistanceX = 0.5+ UnitLengthFromCenterPointX;
	double PseudoCenterTileDistanceY =-0.5+ UnitLengthFromCenterPointY;

	GIS_ERROR(StaticStreamer);
	FVector2D CameraOffset = StaticStreamer->GetCameraOffset();
	// Camera offset is -1,-1 for BottomLeft (-1,1) for TopLeft, (1,1) for TopRight, (1,-1) for BottomRight
	FVector2D LocalizedCameraOffset = FVector2D((CameraOffset.X/2)*InStreamingConfig.CameraGridLengthX,
												(CameraOffset.Y/2)*InStreamingConfig.CameraGridLengthY);
	
	double CenterTileDistanceX=  LocalizedCameraOffset.X + PseudoCenterTileDistanceX;
	double CenterTileDistanceY=  LocalizedCameraOffset.Y + PseudoCenterTileDistanceY;
	
	
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
				FString::Printf(TEXT("Canvas Local Hit: X=%.2f, Y=%.2f"), CenterTileDistanceX, CenterTileDistanceY));
	return FGISPoint(X,Y,0,0);
}

