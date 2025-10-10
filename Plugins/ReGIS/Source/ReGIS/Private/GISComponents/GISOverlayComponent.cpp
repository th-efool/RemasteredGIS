// Fill out your copyright notice in the Description page of Project Settings.


#include "GISComponents/GISOverlayComponent.h"
#include "Kismet/GameplayStatics.h"
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
	if (Paths.Num() > 0)
	{
		RenderPaths();
	}
}

void UGISOverlayComponent::AddMarkerAtWorldLocation(double Latitude, double Longitude)
{
	if (!MarkerWidgetClass)
	{
		GIS_ENSURE_POPUP(MarkerWidgetClass,"GISOverlayComponent: No valid marker widget class attached");
		return;
	}

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

void UGISOverlayComponent::AddPathBetweenPoints(const TArray<FMarkerEntry>& Points)
{
	if (Points.Num() < 2) return;

	if (!ViewportActor){GIS_ENSURE_POPUP(ViewportActor, "GISOverlayComponent: Viewport Actor not initialzied"); return;}

	// Convert all Lat/Lon points to world positions
	TArray<FVector> WorldPts;
	int Zoom = ViewportActor->RenderComponent->GetVisualCenterTileID().ZoomLevel;
	for (const auto& P : Points)
	{
		FVector2D Local = ViewportActor->ConvertLatLongToLocalPoint(P.Latitude, P.Longitude, Zoom);
		FVector World = ViewportActor->ConvertLocalPointToWorldPoint(FVector(Local, 0.2f));
		WorldPts.Add(World);
	}

	// Create spline component
	USplineComponent* Spline = NewObject<USplineComponent>(this);
	Spline->RegisterComponent();
	Spline->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	Spline->SetMobility(EComponentMobility::Static);
	for (int32 i = 0; i < WorldPts.Num(); ++i)
	{
		Spline->AddSplinePoint(WorldPts[i], ESplineCoordinateSpace::World);
	}
	Spline->SetClosedLoop(false);

	
	// Create spline meshes between each segment
	TArray<USplineMeshComponent*> MeshSegments;
	for (int32 i = 0; i < WorldPts.Num() - 1; ++i)
	{
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
		SplineMesh->RegisterComponent();
		SplineMesh->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
		SplineMesh->SetMobility(EComponentMobility::Movable);

		// Assign a simple line mesh (you can use a thin cylinder, or create your own)
		if (LineMesh)
			SplineMesh->SetStaticMesh(LineMesh);

		if (LineMaterial)
			SplineMesh->SetMaterial(0, LineMaterial);


		
		FVector StartPos, StartTangent, EndPos, EndTangent;
		Spline->GetLocationAndTangentAtSplinePoint(i, StartPos, StartTangent, ESplineCoordinateSpace::World);
		Spline->GetLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent, ESplineCoordinateSpace::World);

		SplineMesh->SetForwardAxis(ESplineMeshAxis::Z, true);

		SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
		SplineMesh->SetStartScale(FVector2D(0.1f, 0.1f));
		SplineMesh->SetEndScale(FVector2D(0.1f, 0.1f));

		MeshSegments.Add(SplineMesh);
	}

	FPathEntry PathEntry;
	PathEntry.WorldPoints = WorldPts;
	PathEntry.SplineComp = Spline;
	PathEntry.SplineMeshes = MeshSegments;
	PathEntry.WayPoints = Points;

	Paths.Add(PathEntry);
}


void UGISOverlayComponent::RenderMarkers()
{
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
		FVector WorldLocation = ViewportActor->ConvertLocalPointToWorldPoint(FVector(LocalPoint, 20.0f));
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

void UGISOverlayComponent::RenderPaths()
{
	GIS_ENSURE_POPUP(ViewportActor, "GISOverlayComponent: Viewport Actor not initialized");

	if (Paths.Num() == 0) return;
	auto ClipToBoxEdge = [](const FVector2D& A, const FVector2D& B, float Limit, FVector2D& OutA, FVector2D& OutB) -> bool
	{
		// Liang-Barsky style line clipping to axis-aligned box ±Limit
		float t0 = 0.f, t1 = 1.f;
		FVector2D d = B - A;

		auto Clip = [&](float p, float q) -> bool
		{
			if (FMath::IsNearlyZero(p)) return q >= 0.f;
			float r = q / p;
			if (p < 0.f)
			{
				if (r > t1) return false;
				if (r > t0) t0 = r;
			}
			else if (p > 0.f)
			{
				if (r < t0) return false;
				if (r < t1) t1 = r;
			}
			return true;
		};

		if (Clip(-d.X, A.X + Limit) && Clip(d.X, Limit - A.X) &&
			Clip(-d.Y, A.Y + Limit) && Clip(d.Y, Limit - A.Y))
		{
			OutA = A + t0 * d;
			OutB = A + t1 * d;
			return true;
		}
		return false;
	};
	
	int CurrentZoom = ViewportActor->RenderComponent->GetVisualCenterTileID().ZoomLevel;

	for (FPathEntry& Path : Paths)
	{
		if (!Path.SplineComp) continue;

		// Update spline world positions
		TArray<FVector> NewWorldPts;
		for (const FMarkerEntry& P : Path.WayPoints)
		{
			FVector2D Local = ViewportActor->ConvertLatLongToLocalPoint(P.Latitude, P.Longitude, CurrentZoom);
			FVector World = ViewportActor->ConvertLocalPointToWorldPoint(FVector(Local, 1.0f));
			NewWorldPts.Add(World);
		}

		Path.SplineComp->ClearSplinePoints(false);
		for (const FVector& Pt : NewWorldPts)
			Path.SplineComp->AddSplinePoint(Pt, ESplineCoordinateSpace::World, false);
		Path.SplineComp->UpdateSpline();

		// Update each mesh
		for (int32 i = 0; i < Path.SplineMeshes.Num(); ++i)
		{
			USplineMeshComponent* Mesh = Path.SplineMeshes[i];
			if (!Mesh) continue;

			FVector StartPos, StartTangent, EndPos, EndTangent;
			Path.SplineComp->GetLocationAndTangentAtSplinePoint(i, StartPos, StartTangent, ESplineCoordinateSpace::World);
			Path.SplineComp->GetLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent, ESplineCoordinateSpace::World);

			FVector LocalStartT = ViewportActor->ConvertWorldPointToLocalPoint(StartPos);
			FVector2D LocalStart = FVector2D(LocalStartT.X, LocalStartT.Y);
			FVector LocalEndT = ViewportActor->ConvertWorldPointToLocalPoint(EndPos);
			FVector2D LocalEnd   = FVector2D(LocalEndT.X, LocalEndT.Y);

			FVector2D ClippedStart, ClippedEnd;
			bool bInside = ClipToBoxEdge(LocalStart, LocalEnd, 50.f, ClippedStart, ClippedEnd);

			if (!bInside)
			{
				Mesh->SetVisibility(false);
				continue;
			}

			FVector WorldStart = ViewportActor->ConvertLocalPointToWorldPoint(FVector(ClippedStart, 1.0f));
			FVector WorldEnd   = ViewportActor->ConvertLocalPointToWorldPoint(FVector(ClippedEnd, 1.0f));

			Mesh->SetVisibility(true);
			Mesh->SetStartAndEnd(
				WorldStart,
				(WorldEnd - WorldStart).GetSafeNormal() * 100.f,
				WorldEnd,
				(WorldEnd - WorldStart).GetSafeNormal() * 100.f
			);
		}

		
	}
}

void UGISOverlayComponent::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle;

	// Lambda to execute after 3 seconds
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle, 
		[this]()
		{
			UE_LOG(LogTemp, Warning, TEXT("Lambda triggered after 3 seconds!"));
            
			TArray<FMarkerEntry> Path;

			FMarkerEntry Entry1;
			Entry1.Latitude = 28.605181f;
			Entry1.Longitude = 77.194877f;
			Path.Add(Entry1);
			AddMarkerAtWorldLocation(Entry1.Latitude, Entry1.Longitude);

			FMarkerEntry Entry2;
			Entry2.Latitude = 28.637983f;
			Entry2.Longitude = 77.188761f;
			Path.Add(Entry2);
			AddMarkerAtWorldLocation(Entry2.Latitude, Entry2.Longitude);
			
			FMarkerEntry Entry3;
			Entry3.Latitude = 28.613809f;
			Entry3.Longitude = 77.203717f;
			Path.Add(Entry3);
			AddMarkerAtWorldLocation(Entry3.Latitude, Entry3.Longitude);
			
			FMarkerEntry Entry4;
			Entry4.Latitude = 28.631523f;
			Entry4.Longitude = 77.219853f;
			Path.Add(Entry4);
			AddMarkerAtWorldLocation(Entry4.Latitude, Entry4.Longitude);
			
			FMarkerEntry Entry5;
			Entry5.Latitude = 28.625138f;
			Entry5.Longitude = 77.239208f;
			Path.Add(Entry5);
			AddMarkerAtWorldLocation(Entry5.Latitude, Entry5.Longitude);
			
			FMarkerEntry Entry6;
			Entry6.Latitude = 28.614345f;
			Entry6.Longitude = 77.231226;
			Path.Add(Entry6);
			AddMarkerAtWorldLocation(Entry6.Latitude, Entry6.Longitude);


			// Render the path between these points
			AddPathBetweenPoints(Path);
			// Your custom logic here
		}, 
		3.0f,  // Delay in seconds
		false  // false = do not loop
	);
}






