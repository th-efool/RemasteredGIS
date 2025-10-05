#pragma once
#include "DataResource.h"
#include "Utils/GISDataType.h"
#include "DBMS/FGISIdentifier.h"
#include "Utils/GISErrorHandler.h"


struct FGISBaseDataNode 
{
public:
	virtual ~FGISBaseDataNode()= default;

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
	bool ResourceFetched = false;
	template<typename T>
	void SetResource(T* InData)
	{
		if (auto AsSharedTGISData = StaticCastSharedPtr<TGISData<T>>(Resource))
		{
			AsSharedTGISData->SetData(InData);
		}
		else {
			Resource = MakeShared<TGISData<T>>(InData);
			GIS_CHECK_PTR(Resource);
		}
		InData->AddToRoot();

	}

	template<typename T>
	T* GetResource() const
	{
		if (auto AsSharedTGISData = StaticCastSharedPtr<TGISData<T>>(Resource))
		{
			return AsSharedTGISData->GetData();
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
		GIS_CHECK_PTR(InSelf);
		FGISBaseDataNode::Initialize(NodeIdentifier, InSelf);
	}
	
	TWeakPtr<FGISBaseDataNode> ParentNode;
	TSharedPtr<FGISBaseDataNode> ChildNode[4];
	
};