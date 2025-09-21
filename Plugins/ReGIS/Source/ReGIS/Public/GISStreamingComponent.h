// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GISDataType.h"
#include "Components/ActorComponent.h"
#include "API/GISStaticTileFetcher.h"
#include "Streaming/StaticStreaming.h"
#include "GISStreamingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REGIS_API UGISStreamingComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StreamingConfig")
	FGISStreamingConfig InStreamingConfig;
	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StreamingConfig")*/
	FGISTileID InCenterTile;
	// CUSTOM WAY TO FEED IN WITHOUT IT BEING AN UPROPERTY	
	

public:	
	// Sets default values for this component's properties
	UGISStreamingComponent();

	// STREAMS
public:
	TArray<StaticStreaming*> StaticStreaming;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void InitStreaming();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
};
