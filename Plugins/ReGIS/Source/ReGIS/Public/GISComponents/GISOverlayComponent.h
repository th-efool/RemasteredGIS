
#pragma once

#include "CoreMinimal.h"
#include "Streaming//PathStreamer.h"

#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GISComponents/GISComponentBase.h"
#include "Utils/GISNavigationDatatype.h"
#include "GISOverlayComponent.generated.h"

class AGISStaticTileViewportActor;


USTRUCT(BlueprintType)
struct FMarkerEntry
{
	GENERATED_BODY()

	UPROPERTY()
	UWidgetComponent* WidgetComp=nullptr;
	UPROPERTY()
	double Latitude=0;
	UPROPERTY()
	double Longitude=0;
	UPROPERTY()
	FVector WorldLocation=FVector(0.f,0.f,0.f);
};

USTRUCT()
struct FPathEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FMarkerEntry> WayPoints;

	UPROPERTY()
	uint32 PathID=0;

	UPROPERTY()
	USplineComponent* SplineComp = nullptr;

	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshes;
};

UCLASS()
class REGIS_API UGISOverlayComponent : public UGISComponentBase
{
	GENERATED_BODY()
public:
	UGISOverlayComponent()=default;
	virtual void StartupComponent() override;
	// Called every frame to update screen-space positions
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	AGISStaticTileViewportActor* ViewportActor;

// MARKERS	
	// The widget class to use for markers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> MarkerWidgetClass;
private:
	UPROPERTY()
	TArray<FMarkerEntry> Markers;
	void RenderMarkers();
public:
	// Adds a new marker widget at world location
	UFUNCTION(BlueprintCallable)
	void AddMarkerAtWorldLocation(double Latitude, double Longitude);

	
// PATHS

public:
	PathStreamer* PathStreamerObj;
	void StartJourney(ParamsNavigationFetcher JounreyParams, int32 inJourneyID);

	
	UFUNCTION()
	void AddPathBetweenPoints(const TArray<FMarkerEntry>& Points);

private:
	UPROPERTY()
	TArray<FPathEntry> Paths;
	void RenderPaths();

public:
	UPROPERTY(EditAnywhere, Category="Overlay|Paths")
	UStaticMesh* LineMesh;

	UPROPERTY(EditAnywhere, Category="Overlay|Paths")
	UMaterialInterface* LineMaterial;

virtual void BeginPlay() override;
	
};
