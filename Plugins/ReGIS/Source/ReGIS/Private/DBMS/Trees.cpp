#include "DBMS/Trees.h"





TSharedPtr<FGISBaseDataNode> BaseTree::GetNode(const FGISIdentifier& NodeID)
{
	if ( TSharedPtr<FGISBaseDataNode>* FoundNode = QuadTreeMap.Find(NodeID.Hash))
	{
		return *FoundNode;
	}
	else
	{
		return CreateNode(NodeID);
	};
}


TSharedPtr<FGISBaseDataNode> BaseTree::GetParentNode(
	const ICustomParams ParentIndexingParams)
{
	return GetNode(ComputeParentNodeID());
}

TSharedPtr<FGISBaseDataNode> BaseTree::GetChildNode(
	const ICustomParams ChildIndexingParams)
{
	return GetNode(ComputeChildNodeID(ChildIndexingParams));
}

TSharedPtr<FGISBaseDataNode> QuadTree::CreateNode(const FGISIdentifier& NodeID)
{
	TSharedPtr<FGISQTNode> DerivedPtr = MakeShared<FGISQTNode>();
	TSharedPtr<FGISBaseDataNode> CreatedNode = StaticCastSharedPtr<FGISBaseDataNode>(DerivedPtr);
	CreatedNode->Initialize(NodeID, CreatedNode);
	LinkParentChildNodes(DerivedPtr);
	QuadTreeMap.Add(NodeID.Hash,MoveTemp(CreatedNode)) ;
	return CreatedNode;
}


void QuadTree::LinkParentChildNodes(TSharedPtr<FGISQTNode> Node)
{
	if (auto* ParentNode = QuadTreeMap.Find(ComputeParentID(Node.Get()).Hash))
	{
		TSharedPtr<FGISQTNode>& ParentNodeRef = *ParentNode;
		(ParentNodeRef)->ChildNode[CalculateChildIndex(Node->TileID)] = Node;
		Node->ParentNode = (ParentNodeRef);
	}

	for (int i=0; i<4; i++)
	{
		if (auto* ChildNode = QuadTreeMap.Find(ComputeChildID(Node.Get(), i).Hash))
		{
			TSharedPtr<FGISQTNode>& ChildNodeRef = *ChildNode;
			(ChildNodeRef)->ParentNode = Node;
			Node->ChildNode[i] = (ChildNodeRef);
		}
	}
	
}


