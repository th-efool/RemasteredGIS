// Fill out your copyright notice in the Description page of Project Settings.


#include "GISComponents/GISOverlayComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/GISConversionEngine.h"
#include "Utils/GISErrorHandler.h"
#include "ViewportActor/GISStaticTileViewportActor.h"

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

void UGISOverlayComponent::AddMarkerAtWorldLocation(double Latitude, double Longitude)
{
	GIS_ENSURE_POPUP(MarkerWidgetClass,"GISOverlayComponent: No valid marker widget class attached");
	if (!MarkerWidgetClass){return;}

	UWidgetComponent* WidgetComp = NewObject<UWidgetComponent>(this);
	if (!WidgetComp) return;

	WidgetComp->RegisterComponent();
	WidgetComp->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	WidgetComp->SetWidgetClass(MarkerWidgetClass);
	WidgetComp->SetDrawSize(FVector2D(100.f, 100.f));
	WidgetComp->SetWidgetSpace(EWidgetSpace::World);
	WidgetComp->SetPivot(FVector2D(0.5f, 0.5f));
	WidgetComp->SetTwoSided(true);
	WidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	FMarkerEntry Entry;
	Entry.WidgetComp = WidgetComp;
	Entry.Latitude = Latitude;
	Entry.Longitude = Longitude;
	Markers.Add(Entry);
	
}

void UGISOverlayComponent::RenderMarkers()
{
	GIS_ENSURE_POPUP(ViewportActor,"GISOverlayComponent: ViewportComponent not initialized");
	APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!Cam) return;
	int CurrentZoomLevel = ViewportActor->RenderComponent->GetVisualCenterTileID().ZoomLevel;

	FVector CamLoc = Cam->GetCameraLocation();

	for (auto& Entry : Markers)
	{
		if (!Entry.WidgetComp) continue;
		FVector2D LocalPoint = ViewportActor->ConvertLatLongToLocalPoint(Entry.Latitude, Entry.Longitude, CurrentZoomLevel );
		if (FMath::Abs(LocalPoint.X)>50 || FMath::Abs(LocalPoint.Y)>50 )
		{
			Entry.WidgetComp->SetVisibility(false);
			continue;
		} else
		{
			Entry.WidgetComp->SetVisibility(true);
		}
		FVector WorldLocation = ViewportActor->ConvertLocalPointToWorldPoint(FVector(LocalPoint, 0.0f));
		Entry.WidgetComp->SetWorldLocation(WorldLocation);
		Entry.WorldLocation = WorldLocation;
		FVector ToCamera = CamLoc - Entry.WorldLocation;
		float Distance = ToCamera.Size();

		float Scale = FMath::Clamp(300.f / Distance, 0.2f, 1.0f);
		Entry.WidgetComp->SetWorldScale3D(FVector(Scale));

		// Always face the camera
		Entry.WidgetComp->SetWorldRotation((CamLoc - Entry.WorldLocation).Rotation());
	}

}




