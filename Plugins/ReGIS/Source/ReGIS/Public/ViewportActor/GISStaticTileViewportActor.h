// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GISTileViewportActor.h"
#include  "Utils/GISDataType.h"

#include <utility>

#include "GISComponents/GISStaticTileRendererComponent.h"
#include "GISStaticTileViewportActor.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCanvasClicked, FVector2D, LocalCoord);

UCLASS()
class REGIS_API AGISStaticTileViewportActor : public AGISTileViewportActor
{
	GENERATED_BODY()
	
public:
	AGISStaticTileViewportActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


	// INPUT & USER INTERACTION
public:
	UPROPERTY(BlueprintAssignable, Category="Canvas")
	FOnCanvasClicked OnCanvasClicked;
	UFUNCTION()
	void HandleOnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
	
	// COMPONENTS
private:
	UPROPERTY()
	UGISStaticTileRendererComponent* RenderComponent;

	// UPDATE CONIFGURATION
private:
	void RefreshConfig() override;
	void OnConstruction(const FTransform& Transform) override;    
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	UStaticMesh* TileBaseMeshAsset;    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")    
	UMaterialInterface* TileBaseMaterialAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FGISStreamingConfig InStreamingConfig;  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FInputTileData InputConfigData;


};
