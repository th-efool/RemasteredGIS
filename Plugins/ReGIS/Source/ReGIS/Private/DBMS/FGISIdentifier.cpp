#include "DBMS/FGISIdentifier.h"

FGISIdentifier::FGISIdentifier(): FGISIdentifier(0)
{}

FGISIdentifier::FGISIdentifier(int64 InHash)
{Hash = InHash;}

int8 FGISTreeIdentifier::CalculateTileIntIndexAsChild() const
{
	return 0;
}

int8 FGISTreeIdentifier::CalculateTileIntIndexAsParent() const
{
	return 0;
}

FGISIdentifier FGISTreeIdentifier::ParentTileID(int8 ParentIndex)
{
	return FGISIdentifier();
}

FGISIdentifier FGISTreeIdentifier::ChildTileID(int8 ChildIndex)
{
	return FGISIdentifier();
}

FGISTileID::FGISTileID(): FGISTileID(14,256,256)
{
}

FGISTileID::FGISTileID(int8 InZoomLevel, int32 InX, int32 InY): ZoomLevel(InZoomLevel), X(InX), Y(InY)
{
	Hash = ((uint64)ZoomLevel << 56) | ((uint64)X << 28) | (uint64)Y;
}

FGISTileID::FGISTileID(uint64 InHash)
{
	Hash = InHash;
	ZoomLevel = (InHash >> 56) & 0xFF;
	X = (InHash >> 28) & 0xFFFFFF;
	Y = InHash & 0xFFFFFF;
}

int8 FGISTileID::CalculateTileIntIndexAsChild() const
{
	return (X%2)+(Y%2)*2;
	// even,even =Child1     odd, even = Child2
	// even, odd =CHild3    odd, odd = Child4
}

int8 FGISTileID::CalculateTileIntIndexAsParent() const
{
	return 0;
}

FGISIdentifier FGISTileID::ParentTileID(int8 ParentIndex)
{
	return static_cast<FGISIdentifier>(FGISTileID(ZoomLevel-1, floor(X*0.5) ,floor(Y*0.5) ));
}

FGISIdentifier FGISTileID::ChildTileID(int8 ChildIndex)
{
	check(ChildIndex >= 0 && ChildIndex <= 3);
	switch (ChildIndex)
	{
	case 0:
		// top-left
		return static_cast<FGISIdentifier>(FGISTileID(ZoomLevel + 1, X * 2, Y * 2));
		break;
	case 1:
		// top-right
		return static_cast<FGISIdentifier>(FGISTileID(ZoomLevel + 1, X * 2 + 1, Y * 2));
		break;
	case 2:
		// bottom-left
		return static_cast<FGISIdentifier>(FGISTileID(ZoomLevel + 1, X * 2,Y * 2 + 1));
		break;
	case 3:
		// bottom-right
		return static_cast<FGISIdentifier>(FGISTileID(ZoomLevel + 1, X * 2 + 1, Y * 2 + 1));
		break;
	default:
		return static_cast<FGISIdentifier>(FGISTileID());
	}
}
