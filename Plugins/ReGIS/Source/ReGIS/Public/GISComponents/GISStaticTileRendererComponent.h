// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GISComponentBase.h"
#include "Components/SceneComponent.h"
#include "DBMS/FGISIdentifier.h"
#include "Streaming/StaticStreaming.h"
#include "Utils/GISDataType.h"
#include "GISStaticTileRendererComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REGIS_API UGISStaticTileRendererComponent : public UGISComponentBase
{
	GENERATED_BODY()
public:
	virtual void StartupComponent() override;
// PURE PUBLIC
public:
	void Pan(float& X, float& Y);
	void EnumeratePanShift(float& X, float& Y, FGISTileID& TileID);
	void Zoom(int Delta);

// SEMI PUBLIC
public:
	void FetchVisibleTiles();

public:
	TArray<FGISTileID> VisibleTilesID;
	
	FGISTileID GetLogicalCenterTileID();
	FGISTileID GetVisualCenterTileID();;
	int GetCenterTileIndex() const;
	
	void UpdateLocation(FGISTileID newCentertileID);
	void UpdateLocation(FGISPoint newCenterLongLat);

	UTexture2D* GetRenderTexture() const{return StaticStreamer->GetStreamingTexture();}
	FVector2D GetRawCameraOffset() const{return StaticStreamer->GetCameraOffset();}
public:	
	// Sets default values for this component's properties
	UGISStaticTileRendererComponent();
	
	void RefreshInputConfig(FGISStreamingConfig InStreamingConfig, FInputTileData InputConfigData);
	virtual void RefreshInternalConfiguration() override;
		
	// CONFIGURATION INITIALIZATION
private:
	UPROPERTY(VisibleAnywhere, Category = "StreamingConfig")    
	FGISStreamingConfig StreamingConfig;  
	UPROPERTY(VisibleAnywhere, Category = "StreamingConfig")
	FInputTileData TileConfigData;
	
private:
	
	FGISTileID CenterTileID;
	unsigned int FetchIndex=0;
	void HandleTexture(UTexture2D* Texture,unsigned int fetchIndex, int TileIndex) const;
	/*
	virtual void OnConstruction(const FTransform& Transform);
	*/

private:
	StaticStreaming* StaticStreamer;
	void InitStaticStreaming();

private:	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
//DEBUG TEST
private:
	void TestCameraMovement(float DeltaTime);
};
