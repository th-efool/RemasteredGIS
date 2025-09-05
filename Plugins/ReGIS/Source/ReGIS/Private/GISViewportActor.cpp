// Fill out your copyright notice in the Description page of Project Settings.


#include "GISViewportActor.h"
#include "GISErrorHandler.h"

// Sets default values
AGISViewportActor::AGISViewportActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;
	StreamingManagerComponent = CreateDefaultSubobject<UGISStreamingComponent>(TEXT("UGISStreamingManager"));

	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetupAttachment(RootSceneComponent);
	TileMesh->SetWorldScale3D(FVector(5,5,1));
	
	GIS_HANDLE_IF (TileBaseMaterialAsset)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(TileBaseMaterialAsset, TileMesh);
	}
	GIS_HANDLE_IF (TileBaseMeshAsset)
	{
		TileMesh->SetStaticMesh(TileBaseMeshAsset);
	}
}


#if WITH_EDITOR
void AGISViewportActor:: 
PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	OnConstruction(GetActorTransform());
	GIS_HANDLE_IF (TileBaseMaterialAsset)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(TileBaseMaterialAsset, TileMesh);
	}
	GIS_HANDLE_IF (TileBaseMeshAsset)
	{
		TileMesh->SetStaticMesh(TileBaseMeshAsset);
	}

}
#endif


// Called when actor is constructed or property changes in editor
void AGISViewportActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	StreamingManagerComponent->InStreamingConfig = StreamingConfig;

	CenterTile = FGISTileID(ZoomLevel, CenterX, CenterY);
	StreamingManagerComponent->InCenterTile = CenterTile;
	StreamingManagerComponent->StaticStreaming = new UGISStreamingComponent::AtlasStaticStreaming(StreamingConfig.CameraGridLength, StreamingConfig.CameraGridLength, StreamingConfig.GridLength, StreamingConfig.GridLength, StreamingConfig.TileSize, StreamingConfig.TileSize);

	GIS_HANDLE_IF (TileBaseMaterialAsset)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(TileBaseMaterialAsset, TileMesh);
	}
	GIS_HANDLE_IF (TileBaseMeshAsset)
	{
		TileMesh->SetStaticMesh(TileBaseMeshAsset);
	}

}



// Called when the game starts or when spawned
void AGISViewportActor::BeginPlay()
{
	Super::BeginPlay();
	check(StreamingManagerComponent);
	check(TileMesh);
	GIS_HANDLE_IF (TileBaseMeshAsset)
	{
		TileMesh->SetStaticMesh(TileBaseMeshAsset);
	}

	GIS_HANDLE_IF (DynamicMaterial != nullptr)
	{
		DynamicMaterial->SetTextureParameterValue("BaseColor",StreamingManagerComponent->StaticStreaming->StreamingTexture);
		TileMesh->SetMaterial(0, DynamicMaterial);
	}
}

// Called every frame
void AGISViewportActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	static float x,y=0;
	static float z=1;
	x += 0.01f*z;
	y += 0.012f*z;
	if (x>1 || y>1){z=-1; x=1;y=1; }
	if (x<-1 || y<-1){z=1;x=-1;y=-1;}
	StreamingManagerComponent->StaticStreaming->BuildUpdateAtlas();
	StreamingManagerComponent->StaticStreaming->BuildUpdateStreaming(x,y);


}

