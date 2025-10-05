// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DBMS/FGISIdentifier.h"
#include "Streaming/StaticStreaming.h"
#include "Utils/GISDataType.h"
#include "GISStaticTileRendererComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REGIS_API UGISStaticTileRendererComponent : public USceneComponent
{
	GENERATED_BODY()
// PURE PUBLIC
public:
	void Pan(float& X, float& Y);
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


	
public:	
	// Sets default values for this component's properties
	UGISStaticTileRendererComponent();
	
	void RefreshConfig(FGISStreamingConfig InStreamingConfig, FInputTileData InputConfigData,UStaticMesh* TileBaseMeshAsset,UMaterialInterface* TileBaseMaterialAsset );

		
	// CONFIGURATION INITIALIZATION
private:
	UPROPERTY(VisibleAnywhere, Category = "StreamingConfig")    
	FGISStreamingConfig StreamingConfig;  
	UPROPERTY(VisibleAnywhere, Category = "StreamingConfig")
	FInputTileData TileConfigData;
public:
	UPROPERTY()
	UStaticMeshComponent* TileMesh;
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;
	UPROPERTY()
	UStaticMesh* TileBaseMeshAsset;    
	UPROPERTY()    
	UMaterialInterface* TileBaseMaterialAsset;
	
private:
	
	FGISTileID CenterTileID;
	unsigned int FetchIndex=0;
	void HandleTexture(UTexture2D* Texture,unsigned int fetchIndex, int TileIndex) const;

private:
	StaticStreaming* StaticStreamer;
	void InitStaticStreaming();

private:	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	FGISPoint ConvertLocalPointToGISPoint(FVector2D LocalCoord) const;


//DEBUG TEST
private:
	void TestCameraMovement();
};
