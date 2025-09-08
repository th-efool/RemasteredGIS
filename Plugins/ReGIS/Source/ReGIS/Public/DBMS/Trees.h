#pragma once
#include "DataNodes.h"
#include "GISDataType.h"

class BaseTree
{
public:
	
};


/*
class QuadTree : public BaseTree
{
public:
	TMap<uint64,TSharedPtr<FGISQTNode>> QuadTreeMap; 
	TSharedPtr<FGISQTNode> GetNode(const FGISTileID& TileID); //Creates node if it doesn't exist and adds it to map
public: 
	TSharedPtr<FGISQTNode> GetParentNode(const TSharedPtr<FGISQTNode>& Node);
	TSharedPtr<FGISQTNode> GetChildNode(const TSharedPtr<FGISQTNode>& Node,const int8 ChildIndex);
	
private:
	void LinkParentChildNodes(TSharedPtr<FGISQTNode> Node);

	// HELPER FUNCTIONS
	static FGISTileID ComputeParentID(const FGISTileID TileID);
	static FGISTileID ComputeChildID(const FGISTileID TileID, int8 ChildIndex);
	static FGISTileID ComputeParentID(const FGISQTNode* Node);
	static FGISTileID ComputeChildID(const FGISQTNode* Node, int8 ChildIndex);
	static int8 CalculateChildIndex(const FGISTileID TileID);
		
};*/