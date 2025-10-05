// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewportActor/GISStaticTileViewportActor.h"


AGISStaticTileViewportActor::AGISStaticTileViewportActor()
{
	RenderComponent = CreateDefaultSubobject<UGISStaticTileRendererComponent>(TEXT("RenderComponent"));
	RenderComponent->SetupAttachment(RootSceneComponent);
	
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

	if (RenderComponent->TileMesh){
		RenderComponent->TileMesh->OnClicked.AddDynamic(this, &AGISStaticTileViewportActor::HandleOnClicked);
	}
}


void AGISStaticTileViewportActor::Tick(float DeltaTime)  
{  
	Super::Tick(DeltaTime);  
}


void AGISStaticTileViewportActor::RefreshConfig()
{
	Super::RefreshConfig();
	RenderComponent->RefreshConfig(InStreamingConfig,InputConfigData,TileBaseMeshAsset,TileBaseMaterialAsset);
}

void AGISStaticTileViewportActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshConfig();
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
		if (Hit.GetActor() == this)
		{
			// Convert to local coords
			FVector LocalPoint = RenderComponent->TileMesh->GetComponentTransform().InverseTransformPosition(Hit.ImpactPoint);
			FVector2D Local2D(LocalPoint.X, LocalPoint.Y);

			// Fire delegate
			OnCanvasClicked.Broadcast(Local2D);
			RenderComponent->ConvertLocalPointToGISPoint(Local2D);
		}
	}
}


