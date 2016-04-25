#include "Octree.h"

void Octree::Build(const std::vector<Sphere*> &spheres)
{
	glm::vec3 minCoordinates, maxCoordinates;
	FindBoundingBoxCoordinates(spheres, minCoordinates, maxCoordinates);
	glm::vec3 sceneDimension = maxCoordinates - minCoordinates;
	glm::vec3 octreeCenter = (maxCoordinates - minCoordinates) * 0.5f;
	glm::vec3 octreeHalfDimension = glm::vec3(glm::max(glm::max(sceneDimension.x, sceneDimension.y), sceneDimension.z));

	minCoordinates = octreeCenter - octreeHalfDimension;
	maxCoordinates = octreeCenter + octreeHalfDimension;
	root = new OctreeNode(minCoordinates, maxCoordinates, spheres, false);
	BuildChilds(root, 0);
}


void Octree::BuildChilds(OctreeNode *node, int currentLevel)
{
	glm::vec3 parentDim = node->maxCoordinates - node->minCoordinates;

	glm::vec3 minCoordinates1, maxCoordinates1;
	minCoordinates1 = node->minCoordinates;
	maxCoordinates1 = node->minCoordinates;
	maxCoordinates1.x += parentDim.x / 2;
	maxCoordinates1.y += parentDim.y / 2;
	maxCoordinates1.z += parentDim.z / 2;
	CreateChild(node, minCoordinates1, maxCoordinates1, currentLevel);

	glm::vec3 minCoordinates2, maxCoordinates2;
	minCoordinates2 = minCoordinates1;
	maxCoordinates2 = maxCoordinates1;
	minCoordinates2.x += parentDim.x / 2;
	maxCoordinates2.x += parentDim.x / 2;
	CreateChild(node, minCoordinates2, maxCoordinates2, currentLevel);

	glm::vec3 minCoordinates3, maxCoordinates3;
	minCoordinates3 = minCoordinates1;
	maxCoordinates3 = maxCoordinates1;
	minCoordinates3.y += parentDim.y / 2;
	maxCoordinates3.y += parentDim.y / 2;
	CreateChild(node, minCoordinates3, maxCoordinates3, currentLevel);

	glm::vec3 minCoordinates4, maxCoordinates4;
	minCoordinates4 = minCoordinates1;
	maxCoordinates4 = maxCoordinates1;
	minCoordinates4.x += parentDim.x / 2;
	maxCoordinates4.x += parentDim.x / 2;
	minCoordinates4.y += parentDim.y / 2;
	maxCoordinates4.y += parentDim.y / 2;
	CreateChild(node, minCoordinates4, maxCoordinates4, currentLevel);
	//////////////////////////////////////////////////////////////////////////
	glm::vec3 minCoordinates5, maxCoordinates5;
	minCoordinates5 = minCoordinates1;
	maxCoordinates5 = maxCoordinates1;
	minCoordinates5.z += parentDim.z / 2;
	maxCoordinates5.z += parentDim.z / 2;
	CreateChild(node, minCoordinates5, maxCoordinates5, currentLevel);

	glm::vec3 minCoordinates6, maxCoordinates6;
	minCoordinates6 = minCoordinates5;
	maxCoordinates6 = maxCoordinates5;
	minCoordinates6.x += parentDim.x / 2;
	maxCoordinates6.x += parentDim.x / 2;
	CreateChild(node, minCoordinates6, maxCoordinates6, currentLevel);

	glm::vec3 minCoordinates7, maxCoordinates7;
	minCoordinates7 = minCoordinates5;
	maxCoordinates7 = maxCoordinates5;
	minCoordinates7.y += parentDim.y / 2;
	maxCoordinates7.y += parentDim.y / 2;
	CreateChild(node, minCoordinates7, maxCoordinates7, currentLevel);

	glm::vec3 minCoordinates8, maxCoordinates8;
	minCoordinates8 = minCoordinates5;
	maxCoordinates8 = maxCoordinates5;
	minCoordinates8.x += parentDim.x / 2;
	maxCoordinates8.x += parentDim.x / 2;
	minCoordinates8.y += parentDim.y / 2;
	maxCoordinates8.y += parentDim.y / 2;
	CreateChild(node, minCoordinates8, maxCoordinates8, currentLevel);
	
	node->spheres.clear();
}

void Octree::CreateChild(OctreeNode *node, glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates, int currentLevel)
{
	std::vector<Sphere*> childSpheres;
	FindChildSpheres(node->spheres, childSpheres, minCoordinates, maxCoordinates);
	bool isLeaf = childSpheres.size() <= maxPrimitivePerLeaf || currentLevel >= maxLevel;
	OctreeNode* child = new OctreeNode(minCoordinates, maxCoordinates, childSpheres, isLeaf);
	node->AddChild(*child);
	if(!child->isLeafNode)
	{
		BuildChilds(child, currentLevel + 1);
	}
}


void Octree::FindChildSpheres(const std::vector<Sphere*> &parentSpheres,std::vector<Sphere*> &childSpheres, glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates)
{
	for (int i = 0; i < parentSpheres.size(); i++)
	{
		if (parentSpheres[i]->IsInsideBoundaries(minCoordinates, maxCoordinates))
		{
			childSpheres.push_back(parentSpheres[i]);
		}		
	}
}

void Octree::FindBoundingBoxCoordinates(const std::vector<Sphere*> &spheres, glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates)
{
	minCoordinates = glm::vec3(FLT_MAX);
	maxCoordinates = glm::vec3(-FLT_MAX);

	for (int i = 0; i < spheres.size(); i++)
	{
		if (spheres[i]->boundingBoxMin.x < minCoordinates.x)
		{
			minCoordinates.x = spheres[i]->boundingBoxMin.x;
		}

		if (spheres[i]->boundingBoxMin.y < minCoordinates.y)
		{
			minCoordinates.y = spheres[i]->boundingBoxMin.y;
		}

		if (spheres[i]->boundingBoxMin.z < minCoordinates.z)
		{
			minCoordinates.z = spheres[i]->boundingBoxMin.z;
		}

		if (spheres[i]->boundingBoxMax.x > maxCoordinates.x)
		{
			maxCoordinates.x = spheres[i]->boundingBoxMax.x;
		}

		if (spheres[i]->boundingBoxMax.y > maxCoordinates.y)
		{
			maxCoordinates.y = spheres[i]->boundingBoxMax.y;
		}

		if (spheres[i]->boundingBoxMax.z > maxCoordinates.z)
		{
			maxCoordinates.z = spheres[i]->boundingBoxMax.z;
		}
	}
}