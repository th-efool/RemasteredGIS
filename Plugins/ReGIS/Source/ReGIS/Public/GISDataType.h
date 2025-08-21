// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GISDataType.generated.h"

/**
 * 
 */
UCLASS()
class REGIS_API UGISDataType : public UObject
{
	GENERATED_BODY()
	
};


USTRUCT(BlueprintType)
struct FGISPoint
{
	GENERATED_BODY();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GIS")
	float Latitude;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GIS")
	float Longitude;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GIS")
	float Altitude;
	FGISPoint()
		: Latitude(0.0f), Longitude(0.0f), Altitude(0.0f)
	{
	}
};


USTRUCT(BlueprintType)
struct FGISTileID {
	GENERATED_BODY();
	union {
		struct {
			int8 ZoomLevel : 8;
			int32 X : 28;
			int32 Y : 28;
		};
	
		uint64 Hash;
	};

	FGISTileID()
		: ZoomLevel(0), X(0), Y(0)
	{
	}

	FGISTileID(int8 InZoomLevel, int32 InX, int32 InY)
		: ZoomLevel(InZoomLevel), X(InX), Y(InY)
	{
		Hash = ((uint64)ZoomLevel << 56) | ((uint64)X << 32) | (uint64)Y;
	}
	FGISTileID(uint64 InHash)
		: Hash(InHash)
	{
		ZoomLevel = (InHash >> 56) & 0xFF;
		X = (InHash >> 32) & 0xFFFFFF;
		Y = InHash & 0xFFFFFF;
	}
};

USTRUCT(BlueprintType)
struct FGISStreamingConfig
{
	GENERATED_BODY();
	UPROPERTY(EditAnywhere)
	int32 TileSize = 256;
	UPROPERTY(EditAnywhere)
	int8 GridLength = 4;
	UPROPERTY(EditAnywhere)
	int8 CameraGridLength = 2;

};




// GIS DATA RESOURCE TYPES
class IBaseGISData {
public:
	virtual ~IBaseGISData() {}
	virtual bool IsLoaded() const = 0;
};

template<typename T>
class TGISData : public IBaseGISData
{
	T* Data;
public:
	TGISData(T* InData = nullptr) : Data(InData) {}
	virtual ~TGISData() { delete Data; }

	virtual bool IsLoaded() const override { return Data != nullptr; }
	T* GetData() const { return Data; }
	void SetData(T* InData)
	{
		if (Data != InData) {
			delete Data;
			Data = InData;
		}
	}
};



USTRUCT(BlueprintType)
struct FGISQTNode 
{
	GENERATED_BODY();
	FGISTileID TileID;
	TWeakPtr<FGISQTNode> ParentNode;
	TSharedPtr<FGISQTNode> ChildNode[4];


	TWeakPtr<FGISQTNode> WeakSelf;
	inline void Initialize(FGISTileID InTileID, TSharedPtr<FGISQTNode> InSelf)
	{
		this->TileID = InTileID;
		check(!WeakSelf.IsValid()); // prevent accidental re-assignment
		WeakSelf = InSelf;
	}
	FORCEINLINE bool IsLeaf(){return (!ChildNode[0] && !ChildNode[1] && !ChildNode[2] && !ChildNode[3]);};
	
	// DATA MANAGEMENT
private:
	TSharedPtr<IBaseGISData> Resource;
public:
	template<typename T>
	void SetResource(T* InData)
	{
		if (auto AsSharedTGISData = StaticCastSharedPtr<TGISData<T>>(Resource))
		{
			AsSharedTGISData->SetData(InData);
		}
		else {
			Resource = MakeShared<TGISData<T>>(InData);
		}

	}

	template<typename T>
	T* GetResource() const
	{
		if (auto AsSharedTGISData = StaticCastSharedPtr<TGISData<T>>(Resource))
		{
			return AsSharedTGISData->GetResource();
		}
		return nullptr;
	}

};
