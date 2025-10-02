// Fill out your copyright notice in the Description page of Project Settings.


#include "DBMS/DataManager.h"
#include "ReGIS/Public/Utils/GISErrorHandler.h"


UDataManager::UDataManager()
{
	InitializeComponents();
}


void UDataManager::GetStaticTile(FGISTileID TileID, TFunction<void(UTexture2D*)> callback)
{
    UE_LOG(LogTemp, Display, TEXT("GetStaticTile START: TileID=%d"), (int)TileID.Hash);

    bool Out_NewNodeCreate = false;
    TSharedPtr<FGISBaseDataNode> Node = StaticTileQT->GetNode(TileID, Out_NewNodeCreate);
    GIS_CHECK_PTR(Node);

    if (!Out_NewNodeCreate)
    {
        UE_LOG(LogTemp, Display, TEXT("Node already exists: ResourceFetched=%s"), Node->ResourceFetched ? TEXT("true") : TEXT("false"));

        if (Node->ResourceFetched)
        {
            UE_LOG(LogTemp, Display, TEXT("Resource already fetched: returning immediately"));
            callback(Node->GetResource<UTexture2D>());
        }
        else
        {
            UE_LOG(LogTemp, Display, TEXT("Resource fetch in-flight: queuing callback"));
            PendingCallbacks.FindOrAdd(TileID.Hash).Add(callback);
        }
        return;
    }

    UE_LOG(LogTemp, Display, TEXT("Node not present: creating placeholder resource and queuing callback"));
    PendingCallbacks.FindOrAdd(TileID.Hash).Add(callback);

    ParamsStaticTileFetcher StaticTileParams = ParamsStaticTileFetcher(TileID);
    UE_LOG(LogTemp, Display, TEXT("Making async API call for TileID=%d"), (int)TileID.Hash);

    StaticTileFetcher->MakeApiCall(StaticTileParams, [Node, callback, TileID, this](void* RawData)
    {
        UTexture2D* Texture = static_cast<UTexture2D*>(RawData);
        Node->SetResource(Texture);
        Node->ResourceFetched = true;

        UE_LOG(LogTemp, Display, TEXT("Async API call completed: TileID=%d, invoking queued callbacks"), (int)TileID.Hash);

        if (TArray<TFunction<void(UTexture2D*)>>* Callbacks = PendingCallbacks.Find(TileID.Hash))
        {
            for (auto& CB : *Callbacks)
            {
                CB(Texture);
            }
            PendingCallbacks.Remove(TileID.Hash);
            UE_LOG(LogTemp, Display, TEXT("Pending callbacks cleared for TileID=%d"), (int)TileID.Hash);
        }

        callback(Texture);
    });
}

void UDataManager::InitializeComponents()
{
	StaticTileFetcher = new GISStaticTileFetcher();
	StaticTileQT = new QuadTree();
}
