// Fill out your copyright notice in the Description page of Project Settings.


#include "DBMS/DataManager.h"
#include "ReGIS/Public/Utils/GISErrorHandler.h"


UDataManager::UDataManager()
{
	InitializeComponents();
}


void UDataManager::GetStaticTile(FGISTileID TileID, TFunction<void(UTexture2D*)> callback)
{
    bool Out_NewNodeCreate = false;
    TSharedPtr<FGISBaseDataNode> Node = StaticTileQT->GetNode(TileID, Out_NewNodeCreate);
    GIS_CHECK_PTR(Node);

    if (!Out_NewNodeCreate)
    {
        if (Node->ResourceFetched)
        {
            callback(Node->GetResource<UTexture2D>());
        }
        else
        {
            PendingCallbacks.FindOrAdd(TileID.Hash).Add(callback);
        }
        return;
    }

    PendingCallbacks.FindOrAdd(TileID.Hash).Add(callback);

    ParamsStaticTileFetcher StaticTileParams = ParamsStaticTileFetcher(TileID);

    StaticTileFetcher->MakeApiCall(StaticTileParams, [Node, callback, TileID, this](void* RawData)
    {
        UTexture2D* Texture = static_cast<UTexture2D*>(RawData);
        Node->SetResource(Texture);
        Node->ResourceFetched = true;


        if (TArray<TFunction<void(UTexture2D*)>>* Callbacks = PendingCallbacks.Find(TileID.Hash))
        {
            for (auto& CB : *Callbacks)
            {
                CB(Texture);
            }
            PendingCallbacks.Remove(TileID.Hash);
        }

        callback(Texture);
    });
}

void UDataManager::GetNavigationData(ParamsNavigationFetcher NavParams, TFunction<void(FRoute*)> callback)
{
    if (PendingNavigationCallbacks.Contains(NavParams.fetchID))
    {
        PendingNavigationCallbacks.Add(NavParams.fetchID, callback);
        return;
    }

    PendingNavigationCallbacks.FindOrAdd(NavParams.fetchID, callback);

    
    NavigationFetcher->MakeApiCall(NavParams,[this,NavParams](void* RawRouteData)
    {
        FRoute* Route = static_cast<FRoute*>(RawRouteData);
        if (auto CB = PendingNavigationCallbacks.Find(NavParams.fetchID))
        {
            (*CB)(Route);
        };
    });
}

void UDataManager::InitializeComponents()
{
	StaticTileFetcher = new GISStaticTileFetcher();
	StaticTileQT = new QuadTree();
    NavigationFetcher = new GISNavigationFetcher();
}
