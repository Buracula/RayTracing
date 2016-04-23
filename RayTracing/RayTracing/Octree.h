#pragma once

#include <glm\glm.hpp>
#include "Sphere.h"


struct OctreeNode
{
	bool isLeafNode;
	glm::vec3 minCoordinates;
	glm::vec3 maxCoordinates;

	std::vector<Sphere*> spheres;
	OctreeNode* childNodes;


	OctreeNode(glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates,std::vector<Sphere*> &spheres, bool isLeafNode)
	{
		this->minCoordinates = minCoordinates;
		this->maxCoordinates = maxCoordinates;
		this->isLeafNode = isLeafNode;
		this->spheres = spheres;

		this->childCount = 0;
		childNodes = new OctreeNode[8];
	}

	void AddChild(OctreeNode &node)
	{
		childNodes[childCount] = node;
		childCount++;
	}

private:
	int childCount;
	OctreeNode(){};
};

class Octree
{
	int maxPrimitivePerLeaf;
	OctreeNode *root;

	void FindBoundingBoxCoordinates(const std::vector<Sphere*> &spheres, glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates);
	void BuildChilds(OctreeNode *node);
	void CreateChild(OctreeNode *node, glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates);
	void FindChildSpheres(const std::vector<Sphere*> &parentSpheres, std::vector<Sphere*> &childSpheres, glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates);


public:

	Octree(int maxPrimitivePerLeaf)
	{
		this->maxPrimitivePerLeaf = maxPrimitivePerLeaf;
	}
	OctreeNode *GetRootNode()
	{
		return root;
	}
	void Build(const std::vector<Sphere*> &spheres);
	
};