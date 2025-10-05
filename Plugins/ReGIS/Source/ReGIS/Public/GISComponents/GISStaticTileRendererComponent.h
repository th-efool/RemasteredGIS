// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utils/GISConversionEngine.h"
#include "Streaming/StaticStreaming.h"
#include "Utils/GISDataType.h"
#include "GISStaticTileRendererComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REMASTEREDGIS_API UGISStaticTileRendererComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGISStaticTileRendererComponent();
	StaticStreaming* StaticStreamer;
	void RefreshConfig();
	void InitStaticStreaming();
	FTileMeshInstance TileMeshInstance;
		
	// CONFIGURATION INITIALIZATION
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")    
	FGISStreamingConfig InStreamingConfig;  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FInputTileData InputConfigData;

	

public:
	TArray<FGISTileID> VisibleTilesID;
	FGISTileID GetCenterTileID();;
	int GetCenterTileIndex() const;
	void UpdateLocation(IGISCustomDatatypes& tileLocation);

private:
	FGISTileID CenterTileID;
	unsigned int FetchIndex=0;
	void FetchVisibleTiles();
	void HandleTexture(UTexture2D* Texture,unsigned int fetchIndex, int TileIndex) const;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
