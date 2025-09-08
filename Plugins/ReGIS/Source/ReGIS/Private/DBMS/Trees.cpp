#include "D:\ThisPC\Documents\Unreal Projects\RemasteredGIS\Intermediate\Build\Win64\x64\RemasteredGISEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.ValExpApi.Cpp20.h"
#include "DBMS/Trees.h"



TSharedPtr<FGISQTNode> QuadTree::GetNode(const FGISTileID& TileID)
{
	if (auto entry = QuadTreeMap.Find(TileID.Hash))
	{
		return *entry;
	};
	TSharedPtr<FGISQTNode> CreatedNode = MakeShared<FGISQTNode>();
	CreatedNode->Initialize(TileID, CreatedNode);
	LinkParentChildNodes(CreatedNode);
	QuadTreeMap.Add(TileID.Hash,MoveTemp(CreatedNode)) ;
	return CreatedNode;
}


FGISTileID QuadTree::ComputeParentID(const FGISTileID TileID)
{
	return FGISTileID(TileID.ZoomLevel-1, floor(TileID.X*0.5) ,floor(TileID.Y*0.5) ) ;
}

FGISTileID QuadTree::ComputeChildID(const FGISTileID TileID, int8 ChildIndex)
{
	check(ChildIndex >= 0 && ChildIndex <= 3);
	switch (ChildIndex)
	{
	case 0:
		// top-left
		return FGISTileID(TileID.ZoomLevel + 1, TileID.X * 2,     TileID.Y * 2);
		break;
	case 1:
		// top-right
		return FGISTileID(TileID.ZoomLevel + 1, TileID.X * 2 + 1, TileID.Y * 2);
		break;
	case 2:
		// bottom-left
		return FGISTileID(TileID.ZoomLevel + 1, TileID.X * 2,     TileID.Y * 2 + 1);
		break;
	case 3:
		// bottom-right
		return FGISTileID(TileID.ZoomLevel + 1, TileID.X * 2 + 1, TileID.Y * 2 + 1);
		break;
	default:
		AutoRTFM::Unreachable();
		return TileID;

	}
}

FGISTileID QuadTree::ComputeParentID(const FGISQTNode* Node)
{
	return ComputeParentID(Node->TileID);
	
}

FGISTileID QuadTree::ComputeChildID(const FGISQTNode* Node, int8 ChildIndex)
{
	return ComputeChildID(Node->TileID, ChildIndex);
}

int8 QuadTree::CalculateChildIndex(const FGISTileID TileID)
{
	return (TileID.X%2)+(TileID.Y%2)*2;
	// even,even =Child1     odd, even = Child2
	// even, odd =CHild3    odd, odd = Child4

}


TSharedPtr<FGISQTNode> QuadTree::GetParentNode(const TSharedPtr<FGISQTNode>& Node) 
{
	return GetNode(ComputeParentID(Node->TileID));
}

TSharedPtr<FGISQTNode> QuadTree::GetChildNode(const TSharedPtr<FGISQTNode>& Node,
	const int8 ChildIndex)
{
	return GetNode(ComputeChildID(Node->TileID, ChildIndex));
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

