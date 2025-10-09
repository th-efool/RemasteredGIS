// Fill out your copyright notice in the Description page of Project Settings.


#include "GISComponents/GISOverlayComponent.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CanvasPanelSlot.h"
#include "Utils/GISErrorHandler.h"

void UGISOverlayComponent::StartupComponent()
{
	Super::StartupComponent();
}

void UGISOverlayComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Markers.Num() > 0)
	{
		RenderMarkers();
	}
}

void UGISOverlayComponent::AddMarkerAtWorldLocation(FVector WorldLocation)
{
	GIS_ENSURE_POPUP(MarkerWidgetClass,"GISOverlayComponent: No valid marker widget class attached");
	if (!MarkerWidgetClass){return;}

	UWidgetComponent* WidgetComp = NewObject<UWidgetComponent>(this);
	if (!WidgetComp) return;

	WidgetComp->RegisterComponent();
	WidgetComp->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	WidgetComp->SetWidgetClass(MarkerWidgetClass);
	WidgetComp->SetWorldLocation(WorldLocation);
	WidgetComp->SetDrawSize(FVector2D(100.f, 100.f));
	WidgetComp->SetWidgetSpace(EWidgetSpace::World);
	WidgetComp->SetPivot(FVector2D(0.5f, 0.5f));
	WidgetComp->SetTwoSided(true);
	WidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	FMarkerEntry Entry;
	Entry.WidgetComp = WidgetComp;
	Entry.WorldLocation = WorldLocation;
	Markers.Add(Entry);
	
}

void UGISOverlayComponent::RenderMarkers()
{
	APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!Cam) return;

	FVector CamLoc = Cam->GetCameraLocation();

	for (auto& Entry : Markers)
	{
		if (!Entry.WidgetComp) continue;

		FVector ToCamera = CamLoc - Entry.WorldLocation;
		float Distance = ToCamera.Size();

		float Scale = FMath::Clamp(300.f / Distance, 0.2f, 1.0f);
		Entry.WidgetComp->SetWorldScale3D(FVector(Scale));

		// Always face the camera
		Entry.WidgetComp->SetWorldRotation((CamLoc - Entry.WorldLocation).Rotation());
	}

}




