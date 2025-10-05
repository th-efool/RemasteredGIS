// Fill out your copyright notice in the Description page of Project Settings.


#include "GISComponents/GISStaticTileRendererComponent.h"

#include "Compression/lz4.h"
#include "DBMS/DataManager.h"
#include "Utils/GISErrorHandler.h"

// Sets default values for this component's properties
UGISStaticTileRendererComponent::UGISStaticTileRendererComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TileMeshInstance.TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));  
	TileMeshInstance.TileMesh->SetWorldScale3D(FVector(5,5,1));
	FetchIndex=0;
	TileMeshInstance.TileMesh->SetGenerateOverlapEvents(true);
	TileMeshInstance.TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TileMeshInstance.TileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	TileMeshInstance.TileMesh->SetCollisionObjectType(ECC_WorldStatic);

	TileMeshInstance.TileMesh->SetNotifyRigidBodyCollision(true);
	TileMeshInstance.TileMesh->SetCollisionResponseToAllChannels(ECR_Block);
	TileMeshInstance.TileMesh->bSelectable = true;

	// Let it receive input clicks
	TileMeshInstance.TileMesh->SetEnableGravity(false);
	TileMeshInstance.TileMesh->bEditableWhenInherited = true;
}


FGISTileID UGISStaticTileRendererComponent::GetCenterTileID()
{
	if (VisibleTilesID.Num()>0)
	{
		int mid = (InStreamingConfig.AtlasGridLength - 1) / 2;          // 0-based row/col
		int centerIndex = mid * InStreamingConfig.AtlasGridLength + mid + 1; // 1-based sequential index
		centerIndex-=1;
		return VisibleTilesID[centerIndex];
	}
	return CenterTileID;
}

int UGISStaticTileRendererComponent::GetCenterTileIndex() const
{
	int mid = (InStreamingConfig.AtlasGridLength - 1) / 2;          // 0-based row/col
	int centerIndex = mid * InStreamingConfig.AtlasGridLength + mid + 1; // 1-based sequential index
	centerIndex-=1; //Counting Starts From Zero
	return centerIndex;
}

void UGISStaticTileRendererComponent::UpdateLocation(IGISCustomDatatypes& tileLocation)
{
	if (auto* tileID = dynamic_cast<FGISTileID*>(&tileLocation))
	{
		CenterTileID = *tileID;
	}
	else if (auto* tilePoint = dynamic_cast<FGISPoint*>(&tileLocation))
	{
		auto [x, y] = GISConversionEngine::LatLonToTile(*tilePoint);
		CenterTileID = FGISTileID(tilePoint->Zoom, x, y);
	}
	FetchVisibleTiles();
}

void UGISStaticTileRendererComponent::FetchVisibleTiles()
{
	UDataManager* DataManager= GetWorld()->GetGameInstance()->GetSubsystem<UDataManager>();
	if (DataManager==nullptr){return;}
	
	FetchIndex++;
	unsigned int ThisFetchIndex = FetchIndex;
	if (!StaticStreamer){return;}
	int TopLeftCornerX = CenterTileID.X-0.5*(InStreamingConfig.AtlasGridLength);
	int TopLeftCornerY = CenterTileID.Y-0.5*(InStreamingConfig.AtlasGridLength);
	int Zoom = CenterTileID.ZoomLevel;
	if (TopLeftCornerX<0 || TopLeftCornerY<0 || Zoom<0){return;}
	TWeakObjectPtr<UGISStaticTileRendererComponent> WeakThis(this);

	VisibleTilesID.Empty();
	StaticStreamer->ReInitVisibleTiles(); // Converting All Tiles TO WHITE TILES 
	for (int i=0; i< InStreamingConfig.AtlasGridLength; i++)
	{
		for (int j=0; j< InStreamingConfig.AtlasGridLength; j++)
		{
			int TileIndex = j*(InStreamingConfig.AtlasGridLength)+i;
			FGISTileID TileID = FGISTileID(Zoom, TopLeftCornerX+i, TopLeftCornerY+j);
			VisibleTilesID.Add(TileID);
			TFunction<void(UTexture2D*)> Callback =
			[WeakThis,ThisFetchIndex,TileIndex ](UTexture2D* InTexture)
			{
				if (WeakThis.IsValid())
				{WeakThis->HandleTexture(InTexture,ThisFetchIndex,TileIndex);}
			};
			DataManager->GetStaticTile(TileID, Callback );
		}
	}
}

void UGISStaticTileRendererComponent::HandleTexture(UTexture2D* Texture, unsigned int fetchIndex, int TileIndex) const
{
	if (FetchIndex == fetchIndex)
	{		UE_LOG(LogTemp, Display, TEXT("SUCCESSFUL  FetchIndex: %d , TileIndex: %d"), fetchIndex, TileIndex)
		StaticStreamer->SetVisibleTileIndexed(Texture, TileIndex);
	} else
	{
		UE_LOG(LogTemp, Display, TEXT("FAILED  FetchIndex: %d , TileIndex: %d"), fetchIndex, TileIndex)
	
	}
}

// Called when the game starts
void UGISStaticTileRendererComponent::BeginPlay()
{
	Super::BeginPlay();

	InitStaticStreaming();
	FTimerHandle TempHandle;

	GetWorld()->GetTimerManager().SetTimer(TempHandle, [this]()
	{
		FetchVisibleTiles();
	}, 1.0f, false);
	
	
}


// Called every frame
void UGISStaticTileRendererComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UGISStaticTileRendererComponent::RefreshConfig()
{
	if (InputConfigData.UseLatitudeLongitude)
	{
		FGISPoint Point= FGISPoint(InputConfigData.Latitude,InputConfigData.Longitude,0, InputConfigData.ZoomLevel);
		std::pair<int, int> TileID = GISConversionEngine::LatLonToTile(Point);	
		CenterTileID=FGISTileID(InputConfigData.ZoomLevel, TileID.first, TileID.second);

	} else
	{
		CenterTileID=FGISTileID(InputConfigData.ZoomLevel, InputConfigData.CenterX, InputConfigData.CenterY);
	}
	GIS_HANDLE_IF (TileMeshInstance.TileBaseMaterialAsset)  
	{  
		TileMeshInstance.DynamicMaterial = UMaterialInstanceDynamic::Create(TileMeshInstance.TileBaseMaterialAsset, TileMeshInstance.TileMesh);  
	}  
	GIS_HANDLE_IF (TileMeshInstance.TileBaseMeshAsset)  
	{  
		TileMeshInstance.TileMesh->SetStaticMesh(TileMeshInstance.TileBaseMeshAsset);  
	}
}

void UGISStaticTileRendererComponent::InitStaticStreaming()
{
	StaticStreamer = new StaticStreaming(
		InStreamingConfig.CameraGridLength,InStreamingConfig.CameraGridLength,
		InStreamingConfig.AtlasGridLength, InStreamingConfig.AtlasGridLength,
		InStreamingConfig.TileSize, InStreamingConfig.TileSize
		);
	
	GIS_HANDLE_IF (TileMeshInstance.DynamicMaterial != nullptr)
	{
		TileMeshInstance.DynamicMaterial->SetTextureParameterValue("BaseColor",StaticStreamer->GetStreamingTexture());
		TileMeshInstance.TileMesh->SetMaterial(0, TileMeshInstance.DynamicMaterial);
	}
	StaticStreamer->UpdateAtlas();
	StaticStreamer->UpdateStreaming();
}
