#pragma once

struct FGISIdentifier
{
	FGISIdentifier();
	FGISIdentifier(int64 InHash);;
	virtual ~FGISIdentifier() = default;
	
	int64 Hash;
	
};

struct FGISTreeIdentifier : public FGISIdentifier
{
	static constexpr int8 MaxChildren = 1;
	static constexpr int8 MaxParents  = 1;

	virtual int8 GetMaxChildren() const { return MaxChildren;}
	virtual int8 GetMaxParents()  const { return MaxParents;}
	
	virtual int8 CalculateTileIntIndexAsChild() const = 0;
	virtual int8 CalculateTileIntIndexAsParent() const = 0;
	virtual FGISIdentifier ParentTileID(int8 ParentIndex) =0;
	virtual FGISIdentifier ChildTileID(int8 ChildIndex) = 0;

};

struct FGISTileID :FGISTreeIdentifier {
	static constexpr int8 MaxChildren = 4;
	static constexpr int8 MaxParents  = 1;

	virtual int8 GetMaxChildren() const override { return MaxChildren;}
	virtual int8 GetMaxParents()  const override { return MaxParents;}
	
	union {
		struct {
			int8 ZoomLevel : 8;
			int32 X : 28;
			int32 Y : 28;
		};
	};

	FGISTileID();

	FGISTileID(int8 InZoomLevel, int32 InX, int32 InY);

	FGISTileID(uint64 InHash);

	virtual int8 CalculateTileIntIndexAsChild() const override;;

	virtual int8 CalculateTileIntIndexAsParent() const override;;

	virtual FGISIdentifier ParentTileID(int8 ParentIndex) override;

	virtual FGISIdentifier ChildTileID(int8 ChildIndex) override;
};
