#include "Octree.h"

void Octree::Build(const std::vector<Sphere*> &spheres)
{
	glm::vec3 minCoordinates, maxCoordinates;
	FindBoundingBoxCoordinates(spheres, minCoordinates, maxCoordinates);

	std::vector<Sphere*> nodeSpheres;
	for (int i = 0; i < spheres.size(); i++)
	{
		nodeSpheres.push_back(spheres[i]);
	}

	root = new OctreeNode(minCoordinates, maxCoordinates, nodeSpheres , false);
	BuildChilds(root);
}


void Octree::BuildChilds(OctreeNode *node)
{
	glm::vec3 parentDim = node->maxCoordinates - node->minCoordinates;

	glm::vec3 minCoordinates1, maxCoordinates1;
	minCoordinates1 = node->minCoordinates;
	maxCoordinates1 = node->minCoordinates;
	maxCoordinates1.x += parentDim.x / 2;
	maxCoordinates1.y += parentDim.y / 2;
	maxCoordinates1.z += parentDim.z / 2;
	CreateChild(node, minCoordinates1, maxCoordinates1);

	glm::vec3 minCoordinates2, maxCoordinates2;
	minCoordinates2 = node->minCoordinates;
	maxCoordinates2 = node->minCoordinates;
	minCoordinates2.x += parentDim.x / 2;
	maxCoordinates2.y += parentDim.y / 2;
	maxCoordinates2.z += parentDim.z / 2;
	CreateChild(node, minCoordinates2, maxCoordinates2);

	glm::vec3 minCoordinates3, maxCoordinates3;
	minCoordinates3 = node->minCoordinates;
	maxCoordinates3 = node->minCoordinates;
	minCoordinates3.y += parentDim.y / 2;
	maxCoordinates3.y =  node->maxCoordinates.y;
	maxCoordinates3.x += parentDim.x / 2;
	maxCoordinates3.z += parentDim.z / 2;
	CreateChild(node, minCoordinates3, maxCoordinates3);

	glm::vec3 minCoordinates4, maxCoordinates4;
	minCoordinates4 = node->minCoordinates;
	maxCoordinates4 = node->minCoordinates;
	minCoordinates4.x += parentDim.x / 2;
	minCoordinates4.y += parentDim.y / 2;
	maxCoordinates4.x = node->maxCoordinates.x;
	maxCoordinates4.y = node->maxCoordinates.y;
	maxCoordinates4.z += parentDim.z / 2;
	CreateChild(node, minCoordinates4, maxCoordinates4);

	glm::vec3 minCoordinates5, maxCoordinates5;
	minCoordinates5 = node->minCoordinates;
	maxCoordinates5 = node->minCoordinates;
	minCoordinates5.z += parentDim.z / 2;
	maxCoordinates5.x += parentDim.x / 2;
	maxCoordinates5.y += parentDim.y / 2;
	maxCoordinates5.z =  node->maxCoordinates.z;
	CreateChild(node, minCoordinates5, maxCoordinates5);

	glm::vec3 minCoordinates6, maxCoordinates6;
	minCoordinates6 = node->minCoordinates;
	maxCoordinates6 = node->minCoordinates;
	minCoordinates6.x += parentDim.x / 2;
	minCoordinates6.z += parentDim.z / 2;
	maxCoordinates6.x = node->maxCoordinates.x;
	maxCoordinates6.y += parentDim.y / 2;
	maxCoordinates6.z = node->maxCoordinates.z;
	CreateChild(node, minCoordinates6, maxCoordinates6);

	glm::vec3 minCoordinates7, maxCoordinates7;
	minCoordinates7 = node->minCoordinates;
	maxCoordinates7 = node->minCoordinates;
	minCoordinates7.y += parentDim.y / 2;
	minCoordinates7.z += parentDim.z / 2;;
	maxCoordinates7.y = node->maxCoordinates.y;
	maxCoordinates7.x += parentDim.x / 2;
	maxCoordinates7.z = node->maxCoordinates.z;
	CreateChild(node, minCoordinates7, maxCoordinates7);

	glm::vec3 minCoordinates8, maxCoordinates8;
	minCoordinates8 = node->minCoordinates;
	maxCoordinates8 = node->maxCoordinates;
	minCoordinates8.x += parentDim.x / 2;
	minCoordinates8.y += parentDim.y / 2;
	minCoordinates8.z += parentDim.z / 2;
	CreateChild(node, minCoordinates8, maxCoordinates8);
	
	node->spheres.clear();
}

void Octree::CreateChild(OctreeNode *node, glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates)
{
	std::vector<Sphere*> childSpheres;
	FindChildSpheres(node->spheres, childSpheres, minCoordinates, maxCoordinates);
	OctreeNode* child = new OctreeNode(minCoordinates, maxCoordinates, childSpheres, childSpheres.size() <= maxPrimitivePerLeaf);
	node->AddChild(*child);
	if(!child->isLeafNode)
	{
		BuildChilds(child);
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