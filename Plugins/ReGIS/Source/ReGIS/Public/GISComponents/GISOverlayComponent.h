// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GISComponents/GISComponentBase.h"
#include "GISOverlayComponent.generated.h"

USTRUCT()
struct FMarkerEntry
{
	GENERATED_BODY()

	UPROPERTY()
	UUserWidget* Widget=nullptr;

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
	
	// The widget class to use for markers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> MarkerWidgetClass;

	// Adds a new marker widget at world location
	UFUNCTION(BlueprintCallable)
	void AddMarkerAtWorldLocation(FVector WorldLocation);
	
private:
	UPROPERTY()
	TArray<FMarkerEntry> Markers;
	void RenderMarkers();
};
