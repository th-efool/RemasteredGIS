
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "GISComponents/GISComponentBase.h"
#include "GISOverlayComponent.generated.h"

class AGISStaticTileViewportActor;


USTRUCT()
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
	
	// The widget class to use for markers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> MarkerWidgetClass;

	// Adds a new marker widget at world location
	UFUNCTION(BlueprintCallable)
	void AddMarkerAtWorldLocation(double Latitude, double Longitude);
	
private:
	UPROPERTY()
	TArray<FMarkerEntry> Markers;
	void RenderMarkers();

	
};
