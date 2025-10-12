
#pragma once

#include "CoreMinimal.h"
#include "GISTileViewportActor.h"
#include  "Utils/GISDataType.h"
#include <utility>
#include "GISComponents/GISOverlayComponent.h"
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
	virtual void RefreshConfig() override;
	virtual void OnConstruction(const FTransform& Transform) override;    
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	// RENDERER COMPONENT
public:
	UPROPERTY()
	UGISStaticTileRendererComponent* RenderComponent;
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Canvas")
	UGISOverlayComponent* OverlayComponent;
	
	FGISPoint ConvertLocalPointToGISPoint(FVector2D LocalCoord) const;
	UFUNCTION(BlueprintCallable)
	FVector2D ConvertLatLongToLocalPoint(double Latitude, double Longitude, int Zoom) const;
	FVector ConvertWorldPointToLocalPoint(FVector WorldPoint) const;
	FVector ConvertLocalPointToWorldPoint(FVector LocalCoord) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FGISMeshAssets TileMeshAssets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FGISMeshInstance TileMeshInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FGISStreamingConfig InStreamingConfig;  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FInputTileData InputConfigData;


public:
	double LongitudeStart;
	double LatitudeStart;
	double LongitudeEnd;
	double LatitudeEnd;
	void TestFetchNavigationData();
	void SetNavigationParams(double Longitude, double Latitude);

};
