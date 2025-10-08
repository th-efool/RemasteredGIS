// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewportActor/GISStaticTileViewportActor.h"

#include "Utils/GISConversionEngine.h"
#include "Utils/GISErrorHandler.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Editor/UnrealEd/Public/UnrealEd.h"
#include "Editor/UnrealEd/Public/Editor.h"
#endif


AGISStaticTileViewportActor::AGISStaticTileViewportActor()
{
	RenderComponent = CreateDefaultSubobject<UGISStaticTileRendererComponent>(TEXT("RenderComponent"));
	RenderComponent->SetupAttachment(RootSceneComponent);
	TileMeshInstance.TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMeshInstance.TileMesh->SetupAttachment(RootSceneComponent);
	TileMeshInstance.SetupSceneMesh();
}


void AGISStaticTileViewportActor::Tick(float DeltaTime)  
{  
	Super::Tick(DeltaTime);  
}

void AGISStaticTileViewportActor::BeginPlay()
{
	Super::BeginPlay();
	StartupComponents();
	StartupInputControls();
	
}

void AGISStaticTileViewportActor::StartupComponents()
{
	RenderComponent->StartupComponent();

	if ((TileMeshAssets.MaterialAsset==nullptr) || (TileMeshAssets.MaterialAsset==nullptr)){return;}
	TileMeshInstance.DynamicMaterial = UMaterialInstanceDynamic::Create(TileMeshAssets.MaterialAsset, TileMeshInstance.TileMesh);
	TileMeshInstance.TileMesh->SetStaticMesh(TileMeshAssets.MeshAsset);
	TileMeshInstance.TileMesh->SetMaterial(0, TileMeshInstance.DynamicMaterial);
	UTexture2D* StreamingTexture = RenderComponent ? RenderComponent->GetRenderTexture() : nullptr;
	TileMeshInstance.DynamicMaterial->SetTextureParameterValue("BaseColor", StreamingTexture);
	
}

void AGISStaticTileViewportActor::StartupInputControls()
{
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

#if WITH_EDITOR

void AGISStaticTileViewportActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshConfig();
	// To Ensure it doesn't do infinite recurrsion on pan edge case 
	if ((InStreamingConfig.AtlasGridLength - InStreamingConfig.CameraGridLength) <= 1)
	{
		InStreamingConfig.AtlasGridLength = InStreamingConfig.CameraGridLength + 2;
		UE_LOG(LogTemp, Warning, TEXT("AtlasGridLength auto-corrected to %d to maintain (Atlas - Camera) > 1"),
			InStreamingConfig.AtlasGridLength);

		/*// refresh details panel, so auto-update value is visible
		if (GEditor)
		{GEditor->RedrawAllViewports();}*/
	}
}
#endif

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





