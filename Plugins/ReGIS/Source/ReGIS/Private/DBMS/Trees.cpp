#include "DBMS/Trees.h"





TSharedPtr<FGISBaseDataNode> BaseTree::GetNode(const FGISIdentifier& NodeID)
{
	if ( TSharedPtr<FGISBaseDataNode>* FoundNode = TreeMap.Find(NodeID.Hash))
	{
		return *FoundNode;
	}
	else
	{
		return CreateNode(NodeID);
	};
}


TSharedPtr<FGISBaseDataNode> BaseTree::GetParentNode(TSharedPtr<FGISBaseDataNode> Node,
	const int8 ParentIndex)
{
	FGISIdentifier NodeID = Node->NodeID;
	FGISTreeIdentifier& TreeCastedNodeID = static_cast<FGISTreeIdentifier&>(NodeID);
	FGISIdentifier ParentID = TreeCastedNodeID.ParentTileID(ParentIndex);
	return GetNode(ParentID);
}

TSharedPtr<FGISBaseDataNode> BaseTree::GetChildNode(TSharedPtr<FGISBaseDataNode> Node,
	const int8 ChildIndex)
{
	FGISIdentifier NodeID = Node->NodeID;
	FGISTreeIdentifier& TreeCastedNodeID = static_cast<FGISTreeIdentifier&>(NodeID);
	FGISIdentifier ChildID = TreeCastedNodeID.ParentTileID(ChildIndex);
	return GetNode(ChildID);
}

TSharedPtr<FGISBaseDataNode> BaseTree::CreateNode(const FGISIdentifier& NodeID)
{
	return nullptr;
}


void BaseTree::LinkParentChildNodes(TSharedPtr<FGISTreeNode> Node)
{
	if (!Node.IsValid()){return;}

	FGISTreeIdentifier& TreeCastedNodeID = static_cast<FGISTreeIdentifier&>(Node->NodeID);

	// --- Link Parents ---
	for (int32 ParentIndex = 0; ParentIndex < TreeCastedNodeID.GetMaxParents(); ++ParentIndex)
	{
		int64 ParentHash = TreeCastedNodeID.ParentTileID(ParentIndex).Hash;

		if (TSharedPtr<FGISBaseDataNode>* ParentNodeBase = TreeMap.Find(ParentHash))
		{
			TSharedPtr<FGISTreeNode> ParentNode = StaticCastSharedPtr<FGISTreeNode>(*ParentNodeBase);
			if (ParentNode.IsValid())
			{
				int8 ParentSlot = ParentIndex;
				int8 ChildSlot  = TreeCastedNodeID.CalculateTileIntIndexAsChild();
				// Link both ways
				ParentNode->ChildNode[ChildSlot] = Node;
				Node->ParentNode[ParentSlot]     = ParentNode;
			}
		}
	}

	// --- Link Children ---
	for (int32 ChildIndex = 0; ChildIndex < TreeCastedNodeID.GetMaxChildren(); ++ChildIndex)
	{
		int64 ChildHash = TreeCastedNodeID.ChildTileID(ChildIndex).Hash;
		
		if (TSharedPtr<FGISBaseDataNode>* ChildNodeBase = TreeMap.Find(ChildHash))
		{
			TSharedPtr<FGISTreeNode> ChildNode = StaticCastSharedPtr<FGISTreeNode>(*ChildNodeBase);
			if (ChildNode.IsValid())
			{
				int8 ChildSlot  = ChildIndex;
				int8 ParentSlot = TreeCastedNodeID.CalculateTileIntIndexAsParent();

				// Link both ways
				Node->ChildNode[ChildSlot]        = ChildNode;
				ChildNode->ParentNode[ParentSlot] = Node;
			}
		}
	}
}


BaseTree::~BaseTree()
{
}

QuadTree::~QuadTree()
{
}


TSharedPtr<FGISBaseDataNode> BaseTree::PostCreateNode(TSharedPtr<FGISBaseDataNode>& CreatedNode,
                                                      const FGISIdentifier& NodeID, TSharedPtr<FGISTreeNode>& TreeNode)
{
	CreatedNode->Initialize(NodeID, CreatedNode);
	LinkParentChildNodes(TreeNode);
	TreeMap.Add(NodeID.Hash, MoveTemp(CreatedNode));
	return CreatedNode;
}

TSharedPtr<FGISBaseDataNode> QuadTree::CreateNode(const FGISIdentifier& NodeID)
{
	TSharedPtr<FGISQTNode> DerivedPtr = MakeShared<FGISQTNode>();
	TSharedPtr<FGISTreeNode> TreeNode = StaticCastSharedPtr<FGISTreeNode>(DerivedPtr);
	TSharedPtr<FGISBaseDataNode> CreatedNode = StaticCastSharedPtr<FGISBaseDataNode>(DerivedPtr);
	
	return PostCreateNode(CreatedNode, NodeID, TreeNode);
}

