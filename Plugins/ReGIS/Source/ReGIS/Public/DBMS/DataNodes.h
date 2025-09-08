#pragma once
#include "DataResource.h"
#include "GISDataType.h"


struct FGISBaseDataNode 
{
public:
	virtual ~FGISBaseDataNode();
	
	TWeakPtr<FGISBaseDataNode> WeakSelf;
	
	inline virtual void Initialize(FGISTileID InTileID, TSharedPtr<FGISBaseDataNode> InSelf)
	{
		check(!WeakSelf.IsValid()); // prevent accidental re-assignment
		WeakSelf = InSelf;
	}
	
	// DATA MANAGEMENT
	TSharedPtr<IBaseGISDataResource> Resource;
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


struct FGISQTNode : private FGISBaseDataNode
{
	FGISTileID TileID;

	inline virtual void Initialize(FGISTileID InTileID, TSharedPtr<FGISBaseDataNode> InSelf) override
	{
		FGISBaseDataNode::Initialize(InTileID, InSelf);
		this->TileID = InTileID;
	}
	
	TWeakPtr<FGISBaseDataNode> ParentNode;
	TSharedPtr<FGISQTNode> ChildNode[4];
	FORCEINLINE bool IsLeaf(){return (!ChildNode[0] && !ChildNode[1] && !ChildNode[2] && !ChildNode[3]);};
	
};