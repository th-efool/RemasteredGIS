// Fill out your copyright notice in the Description page of Project Settings.


#include "GISTileViewportActor.h"

// Sets default values
AGISTileViewportActor::AGISTileViewportActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));  
	RootComponent = RootSceneComponent;  
}

void AGISTileViewportActor::RefreshConfig()
{
}


void AGISTileViewportActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshConfig();
}

void AGISTileViewportActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshConfig();
}


// Called when the game starts or when spawned
void AGISTileViewportActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AGISTileViewportActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

