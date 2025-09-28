// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DBMS/FGISIdentifier.h"
#include "GameFramework/Actor.h"
#include "GISTileViewportActor.generated.h"

UCLASS()
class REGIS_API AGISTileViewportActor : public AActor
{
	GENERATED_BODY()
	
public:
	
	// Sets default values for this actor's properties
	AGISTileViewportActor();
	virtual void RefreshConfig();
	void OnConstruction(const FTransform& Transform) override;    
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
	
	UPROPERTY()
	USceneComponent* RootSceneComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

   	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	

};
