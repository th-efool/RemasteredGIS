// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GISTileViewportActor.h"
#include  "Utils/GISDataType.h"
#include "Streaming/StaticStreaming.h"
#include "Utils/GISConversionEngine.h"
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

private:
	UGISStaticTileRendererComponent RenderComponent;
	void TestCameraMovement();
public:
	UPROPERTY(BlueprintAssignable, Category="Canvas")
	FOnCanvasClicked OnCanvasClicked;
	UFUNCTION()
	void HandleOnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
	
private:
	FGISPoint ConvertLocalPointToGISPoint(FVector2D LocalCoord) const;
};
