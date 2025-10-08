// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GISTileViewportActor.h"
#include  "Utils/GISDataType.h"

#include <utility>

#include "GISComponents/GISStaticTileRendererComponent.h"
#include "GISStaticTileViewportActor.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCanvasClicked, FParamsCanvasClickedDelegate, InputParameters);

UCLASS()
class REGIS_API AGISStaticTileViewportActor : public AGISTileViewportActor
{
	GENERATED_BODY()
	
public:
	AGISStaticTileViewportActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void StartupComponents();
	void StartupInputControls();

	// INPUT & USER INTERACTION
public:
	// INTERACTION
	UPROPERTY(BlueprintAssignable, Category="Canvas")
	FOnCanvasClicked OnCanvasClicked;
	UFUNCTION()
	void HandleOnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	
	// UPDATE CONIFGURATION
private:
	void RefreshConfig() override;
	void OnConstruction(const FTransform& Transform) override;    
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);

	// RENDERER COMPONENT
private:
	UPROPERTY()
	UGISStaticTileRendererComponent* RenderComponent;
	FGISPoint ConvertLocalPointToGISPoint(FVector2D LocalCoord) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FGISMeshAssets TileMeshAssets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FGISMeshInstance TileMeshInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FGISStreamingConfig InStreamingConfig;  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FInputTileData InputConfigData;


};
