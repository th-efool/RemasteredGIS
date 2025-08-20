// Fill out your copyright notice in the Description page of Project Settings.


#include "GISViewportActor.h"

// Sets default values
AGISViewportActor::AGISViewportActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	StreamingManagerComponent = CreateDefaultSubobject<UGISStreamingComponent>(TEXT("UGISStreamingManager"));
}


#if WITH_EDITOR
void AGISViewportActor::
PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	OnConstruction(GetActorTransform());
}
#endif


// Called when actor is constructed or property changes in editor
void AGISViewportActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	StreamingManagerComponent->InStreamingConfig = StreamingConfig;

	CenterTile = FGISTileID(ZoomLevel, CenterX, CenterY);
	StreamingManagerComponent->InCenterTile = CenterTile;
}


// Called when the game starts or when spawned
void AGISViewportActor::BeginPlay()
{
	Super::BeginPlay();


}

// Called every frame
void AGISViewportActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

