#pragma once

struct FGISIdentifier
{
	int64 Hash;
};


struct FGISTileID :FGISIdentifier {
	union {
		struct {
			int8 ZoomLevel : 8;
			int32 X : 28;
			int32 Y : 28;
		};
	};

	FGISTileID()
		: FGISTileID(14,256,256)
	{
	}

	FGISTileID(int8 InZoomLevel, int32 InX, int32 InY)
		: ZoomLevel(InZoomLevel), X(InX), Y(InY)
	{
		Hash = ((uint64)ZoomLevel << 56) | ((uint64)X << 28) | (uint64)Y;
	}
	FGISTileID(uint64 InHash)
	{
		Hash = InHash;
		ZoomLevel = (InHash >> 56) & 0xFF;
		X = (InHash >> 28) & 0xFFFFFF;
		Y = InHash & 0xFFFFFF;
	}

	FORCEINLINE int8 CalculateTileIntIndexAsChild() const{
		return (X%2)+(Y%2)*2;
		// even,even =Child1     odd, even = Child2
		// even, odd =CHild3    odd, odd = Child4
	};

	FORCEINLINE FGISTileID ParentTileID()
	{
		return FGISTileID(ZoomLevel-1, floor(X*0.5) ,floor(Y*0.5) );
	}

	FORCEINLINE FGISTileID CalculateChildrentHash(int8 ChildIndex)
	{
		check(ChildIndex >= 0 && ChildIndex <= 3);
		switch (ChildIndex)
		{
		case 0:
			// top-left
			return FGISTileID(ZoomLevel + 1, X * 2,     Y * 2);
			break;
		case 1:
			// top-right
			return FGISTileID(ZoomLevel + 1, X * 2 + 1, Y * 2);
			break;
		case 2:
			// bottom-left
			return FGISTileID(ZoomLevel + 1, X * 2,     Y * 2 + 1);
			break;
		case 3:
			// bottom-right
			return FGISTileID(ZoomLevel + 1, X * 2 + 1, Y * 2 + 1);
			break;
		default:
			return FGISTileID();
		}
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
