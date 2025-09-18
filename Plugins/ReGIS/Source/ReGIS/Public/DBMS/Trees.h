#pragma once
#include "DataNodes.h"
#include "GISDataType.h"

class BaseTree
{
public:
	TMap<uint64,TSharedPtr<FGISBaseDataNode>> QuadTreeMap;
	
	TSharedPtr<FGISBaseDataNode> GetNode(const FGISIdentifier& NodeID); 
	virtual TSharedPtr<FGISBaseDataNode> CreateNode(const FGISIdentifier& NodeID) =0;
public: 
	TSharedPtr<FGISBaseDataNode> GetParentNode(const ICustomParams ParentIndex);
	TSharedPtr<FGISBaseDataNode> GetChildNode(const ICustomParams ChildIndex);
	
protected:
	virtual void LinkParentChildNodes(TSharedPtr<FGISQTNode> Node) =0;
	~BaseTree();
};
 

class QuadTree : public BaseTree
{
public:
	virtual TSharedPtr<FGISBaseDataNode> CreateNode(const FGISIdentifier& NodeID) override; //Creates node if it doesn't exist and adds it to map
private:
	virtual void LinkParentChildNodes(TSharedPtr<FGISQTNode> Node) override;
};