#pragma once
#include "DataResource.h"
#include "GISDataType.h"


struct FGISBaseDataNode 
{
public:
	virtual ~FGISBaseDataNode();

	FGISIdentifier NodeID;
	TWeakPtr<FGISBaseDataNode> WeakSelf;
	
	inline virtual void Initialize(FGISIdentifier InNodeIdentifier, TSharedPtr<FGISBaseDataNode> InSelf)
	{
		NodeID = InNodeIdentifier;
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

struct FGISTreeNode : public FGISBaseDataNode
{
	public:
	TArray<TWeakPtr<FGISBaseDataNode>> ParentNode;
	TArray<TSharedPtr<FGISBaseDataNode>> ChildNode;
	

	/*
	FORCEINLINE bool IsLeaf(){return (!ChildNode[0] && !ChildNode[1] && !ChildNode[2] && !ChildNode[3]);};
*/
};

struct FGISQTNode : public FGISTreeNode
{

	inline virtual void Initialize(FGISIdentifier NodeIdentifier, TSharedPtr<FGISBaseDataNode> InSelf) override
	{
		FGISBaseDataNode::Initialize(NodeIdentifier, InSelf);
	}
	
	TWeakPtr<FGISBaseDataNode> ParentNode;
	TSharedPtr<FGISBaseDataNode> ChildNode[4];
	
};a