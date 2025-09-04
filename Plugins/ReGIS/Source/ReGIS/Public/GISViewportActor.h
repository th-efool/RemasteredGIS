// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GISDataType.h"
#include "GISStreamingComponent.h"
#include "GISViewportActor.generated.h"

UCLASS()
class REGIS_API AGISViewportActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGISViewportActor();

	// Internal functions
	void InitializeComponents();
	void InitializeStreamingSystem();
	void UpdateMaterialTexture();
	void OnConstruction(const FTransform& Transform) override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
	
     // Systems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UGISStreamingComponent* StreamingManagerComponent;

	// TEMP (CREATE SEPERATE COMPONENT STATIC RENDER TILE!)
	UPROPERTY()
    UMaterialInstanceDynamic* DynamicMaterial;


	// Assets & Initializers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* TileBaseMeshAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UMaterialInterface* TileBaseMaterialAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	FGISStreamingConfig StreamingConfig;



	// Tile Data
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
	FGISTileID CenterTile{ 14,100,100 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	int ZoomLevel = 14;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	int CenterX = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StreamingConfig")
	int CenterY = 100;

	// Configuration Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* RootSceneComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TileMesh;

 protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
