#pragma once
#include "DataNodes.h"
#include "Utils/GISDataType.h"

class BaseTree
{
public:
	BaseTree() = default;
	virtual ~BaseTree() = default;  

	
	
	TMap<uint64,TSharedPtr<FGISBaseDataNode>> TreeMap;
	
	TSharedPtr<FGISBaseDataNode> GetNode(const FGISIdentifier& NodeID, bool& Out_NewNodeCreate); 
	TSharedPtr<FGISBaseDataNode> GetParentNode(TSharedPtr<FGISBaseDataNode> Node,
	const int8 ParentIndex);
	TSharedPtr<FGISBaseDataNode> GetChildNode(TSharedPtr<FGISBaseDataNode> Node,
	const int8 ChildIndex);
protected:
	virtual TSharedPtr<FGISBaseDataNode> CreateNode(const FGISIdentifier& NodeID) = 0;
	TSharedPtr<FGISBaseDataNode> PostCreateNode(TSharedPtr<FGISBaseDataNode>& CreatedNode,
		const FGISIdentifier& NodeID, TSharedPtr<FGISTreeNode>& TreeNode);
private:
	void LinkParentChildNodes(TSharedPtr<FGISTreeNode> Node);
};
 

class QuadTree : public BaseTree
{
public:
	QuadTree() = default;
	~QuadTree() override = default;

private:
	virtual TSharedPtr<FGISBaseDataNode> CreateNode(const FGISIdentifier& NodeID) override; //Creates node if it doesn't exist and adds it to map
};