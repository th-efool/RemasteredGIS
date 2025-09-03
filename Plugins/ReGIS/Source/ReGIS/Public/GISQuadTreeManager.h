// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GISQuadTreeManager.generated.h"

struct FGISQTNode;
struct FGISTileID;
/**
 * 
 */
UCLASS()
class REGIS_API UGISQuadTreeManager : public UObject
{
	GENERATED_BODY()


	class QuadTree
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
		
	};

	
};
