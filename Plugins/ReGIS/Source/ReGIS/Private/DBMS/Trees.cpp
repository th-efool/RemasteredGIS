#include "DBMS/Trees.h"

#include "Utils/GISErrorHandler.h"


TSharedPtr<FGISBaseDataNode> BaseTree::GetNode(const FGISIdentifier& NodeID, bool& Out_NewNodeCreate)
{
	TSharedPtr<FGISBaseDataNode> NodePtr;
	if ( TSharedPtr<FGISBaseDataNode>* FoundNode = TreeMap.Find(NodeID.Hash))
	{
		Out_NewNodeCreate = false;
		NodePtr = *FoundNode;
	}
	else
	{
		Out_NewNodeCreate = true;
		NodePtr= CreateNode(NodeID);
		GIS_CHECK_PTR(NodePtr);
		
	};
	return NodePtr;
}






TSharedPtr<FGISBaseDataNode> BaseTree::GetParentNode(TSharedPtr<FGISBaseDataNode> Node,
                                                     const int8 ParentIndex)
{
	FGISIdentifier NodeID = Node->NodeID;
	FGISTreeIdentifier& TreeCastedNodeID = static_cast<FGISTreeIdentifier&>(NodeID);
	FGISIdentifier ParentID = TreeCastedNodeID.ParentTileID(ParentIndex);
	bool Out_NewNodeCreate;
	return GetNode(ParentID, Out_NewNodeCreate);
}

TSharedPtr<FGISBaseDataNode> BaseTree::GetChildNode(TSharedPtr<FGISBaseDataNode> Node,
	const int8 ChildIndex)
{
	FGISIdentifier NodeID = Node->NodeID;
	FGISTreeIdentifier& TreeCastedNodeID = static_cast<FGISTreeIdentifier&>(NodeID);
	FGISIdentifier ChildID = TreeCastedNodeID.ParentTileID(ChildIndex);
	bool Out_NewNodeCreate;
	return GetNode(ChildID,Out_NewNodeCreate);
}

TSharedPtr<FGISBaseDataNode> BaseTree::CreateNode(const FGISIdentifier& NodeID)
{
	checkNoEntry(); // Safety: should never be called
	return nullptr;
}

void BaseTree::LinkParentChildNodes(TSharedPtr<FGISTreeNode> Node)
{/*
	if (!Node.IsValid())
	{
		GIS_WARN_MSG(false, "LinkParentChildNodes called with invalid Node!");
		return;
	}




	
	FGISTreeIdentifier& TreeCastedNodeID = static_cast<FGISTreeIdentifier&>(Node->NodeID);


	// --- Link Parents ---
	for (int32 ParentIndex = 0; ParentIndex < TreeCastedNodeID.GetMaxParents(); ++ParentIndex)
	{
		int64 ParentHash = TreeCastedNodeID.ParentTileID(ParentIndex).Hash;

		TSharedPtr<FGISBaseDataNode>* ParentNodeBase = TreeMap.Find(ParentHash);
		if (!ParentNodeBase)
		{
			GIS_DEBUG_MSG(false, FString::Printf(TEXT("Parent node with hash %lld not found in TreeMap."), ParentHash));
			continue;
		}

		TSharedPtr<FGISTreeNode> ParentNode = StaticCastSharedPtr<FGISTreeNode>(*ParentNodeBase);
		if (!ParentNode.IsValid())
		{
			GIS_WARN_MSG(false, FString::Printf(TEXT("ParentNode is invalid for hash %lld."), ParentHash));
			continue;
		}

		
		int8 ParentSlot = ParentIndex;
		int8 ChildSlot  = TreeCastedNodeID.CalculateTileIntIndexAsChild();

		// Link both ways
		Node->ParentNode[ParentSlot] = ParentNode;
		ParentNode->ChildNode[ChildSlot] = Node;

		GIS_DEBUG_MSG(false, FString::Printf(
			TEXT("Linked Node %lld to Parent %lld at slots Child:%d Parent:%d"),
			Node->NodeID.Hash, ParentNode->NodeID.Hash, ChildSlot, ParentSlot));
	}

	// --- Link Children ---
	for (int32 ChildIndex = 0; ChildIndex < TreeCastedNodeID.GetMaxChildren(); ++ChildIndex)
	{
		int64 ChildHash = TreeCastedNodeID.ChildTileID(ChildIndex).Hash;

		TSharedPtr<FGISBaseDataNode>* ChildNodeBase = TreeMap.Find(ChildHash);
		if (!ChildNodeBase)
		{
			GIS_DEBUG_MSG(false, FString::Printf(TEXT("Child node with hash %lld not found in TreeMap."), ChildHash));
			continue;
		}

		TSharedPtr<FGISTreeNode> ChildNode = StaticCastSharedPtr<FGISTreeNode>(*ChildNodeBase);
		if (!ChildNode.IsValid())
		{
			GIS_WARN_MSG(false, FString::Printf(TEXT("ChildNode is invalid for hash %lld."), ChildHash));
			continue;
		}

	

		int8 ChildSlot  = ChildIndex;
		int8 ParentSlot = TreeCastedNodeID.CalculateTileIntIndexAsParent();

		// Link both ways
		Node->ChildNode[ChildSlot]        = ChildNode;
		ChildNode->ParentNode[ParentSlot] = Node;

		GIS_DEBUG_MSG(false, FString::Printf(
			TEXT("Linked Node %lld to Child %lld at slots Child:%d Parent:%d"),
			Node->NodeID.Hash, ChildNode->NodeID.Hash, ChildSlot, ParentSlot));
	}*/
}




TSharedPtr<FGISBaseDataNode> BaseTree::PostCreateNode(TSharedPtr<FGISBaseDataNode>& CreatedNode,
                                                      const FGISIdentifier& NodeID, TSharedPtr<FGISTreeNode>& TreeNode)
{
	CreatedNode->Initialize(NodeID, CreatedNode);
	LinkParentChildNodes(TreeNode);
	return TreeMap.Add(NodeID.Hash, MoveTemp(CreatedNode));
}



TSharedPtr<FGISBaseDataNode> QuadTree::CreateNode(const FGISIdentifier& NodeID)
{
	TSharedPtr<FGISQTNode> DerivedPtr = MakeShared<FGISQTNode>();
	GIS_CHECK_PTR(DerivedPtr);
	TSharedPtr<FGISTreeNode> TreeNode = StaticCastSharedPtr<FGISTreeNode>(DerivedPtr);
	GIS_CHECK_PTR(TreeNode);
	TSharedPtr<FGISBaseDataNode> CreatedNode = StaticCastSharedPtr<FGISBaseDataNode>(DerivedPtr);
	GIS_CHECK_PTR(CreatedNode);

	
	return PostCreateNode(CreatedNode, NodeID, TreeNode);
}

