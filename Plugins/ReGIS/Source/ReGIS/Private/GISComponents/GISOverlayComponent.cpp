// Fill out your copyright notice in the Description page of Project Settings.


#include "GISComponents/GISOverlayComponent.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/PlayerController.h"
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
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	UUserWidget* Marker = CreateWidget<UUserWidget>(PC, MarkerWidgetClass);
	if (Marker)
	{
		Marker->AddToViewport();
		FMarkerEntry Entry;
		Entry.Widget = Marker;
		Entry.WorldLocation = WorldLocation;
		Markers.Add(Entry);
	}
}

void UGISOverlayComponent::RenderMarkers()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	for (auto& Entry : Markers)
	{
		FVector2D ScreenPos;
		bool bOnScreen =
			UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
				PC, Entry.WorldLocation, ScreenPos, false);

		if (bOnScreen)
		{
			// Move the widget directly
			Entry.Widget->SetRenderTranslation(ScreenPos);
			Entry.Widget->SetVisibility(ESlateVisibility::Visible);
						
			float Distance = FVector::Dist(PC->PlayerCameraManager->GetCameraLocation(), Entry.WorldLocation);
			
			float Scale = FMath::Clamp(500.0f / Distance, 0.05f, 1.0f);
			Entry.Widget->SetRenderScale(FVector2D(Scale, Scale));
		}
		else
		{
			Entry.Widget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

}


