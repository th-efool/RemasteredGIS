// Fill out your copyright notice in the Description page of Project Settings.


#include "GISStreamingComponent.h"

#include "GISErrorHandler.h"
#include "API/GISStaticTileFetcher.h"

// Sets default values for this component's properties
UGISStreamingComponent::UGISStreamingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}



// Called when the game starts
void UGISStreamingComponent::BeginPlay()
{
	Super::BeginPlay();
	// FETCH TILES FIRST THEN AFTER AN INITIAL TIMER RUN THIS

	/*
	UGISStreamingComponent::InitStreaming();
	*/


}

void UGISStreamingComponent::InitStreaming()
{
	/*
	float x=0,y=0;
	if (StaticStreaming)
	{
		StaticStreaming->BuildUpdateAtlas();
		StaticStreaming->BuildUpdateStreaming(x,y);
	
	}*/

}



// Called every frame
void UGISStreamingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{

	// ...
}





