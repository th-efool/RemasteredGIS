// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewportActor/GISStaticTileViewportActor.h"

#include "Utils/GISConversionEngine.h"
#include "Utils/GISErrorHandler.h"


AGISStaticTileViewportActor::AGISStaticTileViewportActor()
{
	RenderComponent = CreateDefaultSubobject<UGISStaticTileRendererComponent>(TEXT("RenderComponent"));
	RenderComponent->SetupAttachment(RootSceneComponent);
	TileMeshInstance.TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMeshInstance.TileMesh->SetupAttachment(RootSceneComponent);
	TileMeshInstance.SetupSceneMesh();
}



void AGISStaticTileViewportActor::BeginPlay()
{
	Super::BeginPlay();
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
	if (TileMeshInstance.TileMesh)
	{
		TileMeshInstance.TileMesh->OnClicked.AddDynamic(this, &AGISStaticTileViewportActor::HandleOnClicked);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TileMeshInstance.TileMesh is null in BeginPlay!"));
	}


	FTimerHandle TempHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TempHandle,
		[this](){
		
	
			

			// Safely create dynamic material using your macro
			GIS_HANDLE_IF(TileMeshAssets.MaterialAsset && TileMeshInstance.TileMesh)
			{
				TileMeshInstance.DynamicMaterial = UMaterialInstanceDynamic::Create(TileMeshAssets.MaterialAsset, TileMeshInstance.TileMesh);
				if (!TileMeshInstance.DynamicMaterial)
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to create DynamicMaterial!"));
				}
			}

			// Safely set mesh and material
			GIS_HANDLE_IF(TileMeshAssets.MeshAsset && TileMeshInstance.TileMesh)
			{
				TileMeshInstance.TileMesh->SetStaticMesh(TileMeshAssets.MeshAsset);

				if (TileMeshInstance.DynamicMaterial)
				{
					TileMeshInstance.TileMesh->SetMaterial(0, TileMeshInstance.DynamicMaterial);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("DynamicMaterial is null, cannot apply to TileMesh!"));
				}
			}


			// Set streaming texture safely
			UTexture2D* StreamingTexture = RenderComponent ? RenderComponent->GetRenderTexture() : nullptr;
			if (TileMeshInstance.DynamicMaterial && StreamingTexture)
			{
				TileMeshInstance.DynamicMaterial->SetTextureParameterValue("BaseColor", StreamingTexture);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("DynamicMaterial or StreamingTexture is null in BeginPlay!"));
			}



	},
		1.0f, false
		);

	
	
	
}


void AGISStaticTileViewportActor::Tick(float DeltaTime)  
{  
	Super::Tick(DeltaTime);  
}

void AGISStaticTileViewportActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshConfig();
}

void AGISStaticTileViewportActor::RefreshConfig()
{
	Super::RefreshConfig();
	TileMeshInstance.Refresh(TileMeshAssets);
	RenderComponent->RefreshInputConfig(InStreamingConfig,InputConfigData);
}

void AGISStaticTileViewportActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshConfig();
}

void AGISStaticTileViewportActor::HandleOnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	// Get hit under cursor from player controller
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	FHitResult Hit;
	if (PC->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
	{
		if (Hit.GetActor() != this){ return;}
		FVector LocalPoint = TileMeshInstance.TileMesh->GetComponentTransform().InverseTransformPosition(Hit.ImpactPoint);
		FGISPoint GISPoint = ConvertLocalPointToGISPoint(FVector2D(LocalPoint.X, LocalPoint.Y));
		// Fire delegate
		FParamsCanvasClickedDelegate CanvasClickedDelegateParams;
		CanvasClickedDelegateParams.LocalHitCoord = LocalPoint;
		CanvasClickedDelegateParams.Latitude = GISPoint.Latitude;
		CanvasClickedDelegateParams.Longitude= GISPoint.Longitude;
		OnCanvasClicked.Broadcast(CanvasClickedDelegateParams);
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
	double unitLenFromCenterX = scaledX * (0.5f*InStreamingConfig.CameraGridLength);
	double unitLenFromCenterY = scaledY * (0.5*InStreamingConfig.CameraGridLength);

	// 4) [Reserved] pseudo center tile distance (relative to center tile top-left)
	double CenterTileTLX = (InStreamingConfig.AtlasGridLength%2 ==1) ? -0.5f : -1.f; 
	double CenterTileTLY = (InStreamingConfig.AtlasGridLength%2 ==1) ? 0.5f : 1.f;

	// 5) Camera offsets (raw + localized)
	FVector2D cameraOffsetRaw = RenderComponent->GetRawCameraOffset();
	float MaxPossibleTileOffsetX= (InStreamingConfig.AtlasGridLength-InStreamingConfig.CameraGridLength)/2.f;
	float MaxPossibleTileOffsetY= (InStreamingConfig.AtlasGridLength-InStreamingConfig.CameraGridLength)/2.f;
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

	FGISTileID centerTileID = RenderComponent->GetVisualCenterTileID();
	std::pair<double, double> LatLong = GISConversionEngine::TileToLatLon(centerTileID, FVector2D(centerTileDistanceX,-centerTileDistanceY));

	FString Msg = FString::Printf(TEXT("Lat: %.6f, Lon: %.6f"), LatLong.first, LatLong.second);

	// Print to screen for 5 seconds in green
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, Msg);

	// 9) Return the computed GIS point
	return FGISPoint(LatLong.first, LatLong.second, 0, centerTileID.ZoomLevel);
}





