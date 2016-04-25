#pragma once
#include <algorithm>
#include <glm\glm.hpp>
#include "Sphere.h"

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};


struct IntersectionPoint
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec4 color;
};

struct Light
{
	glm::vec3 position;
	glm::vec4 color;

	Light()
	{
		position = glm::vec3(0);
		color = glm::vec4(1);
	}
};

__inline bool RaySphereIntersection(const Sphere &sphere, const Ray &ray, float &t)
{
	glm::vec3 o_c = ray.origin - sphere.center;
	float dDotD = glm::dot(ray.direction, ray.direction);

	float delta = glm::pow(glm::dot(ray.direction, (o_c)), 2)
		- dDotD *(glm::dot(o_c, o_c) - pow(sphere.radius, 2));

	if (delta < 0)
	{
		return false;
	}
	else
	{
		float t1 = (-glm::dot(ray.direction, o_c) + glm::sqrt(delta)) / dDotD;
		float t2 = (-glm::dot(ray.direction, o_c) - glm::sqrt(delta)) / dDotD;

		float minRoot;
		float maxRoot;
		if (t1 > t2)
		{
			minRoot = t2;
			maxRoot = t1;
		}
		else
		{
			minRoot = t2;
			maxRoot = t1;
		}

		if (minRoot < 0)
		{
			if (maxRoot < 0)
			{
				return false;
			}
			else
			{
				t = maxRoot;
				return true;
			}
		}
		else
		{
			t = minRoot;
			return true;
		}

		// 		if(t1 < t2)
		// 		{
		// 			if(t1 < 0)
		// 			{
		// 				if(t2 < 0)
		// 				{
		// 					return false;
		// 				}
		// 				else
		// 				{
		// 					t = t2;
		// 					return true;
		// 				}
		// 			}
		// 			else
		// 			{
		// 				t = t1;
		// 				return true;
		// 			}
		// 		}
		// 		else
		// 		{
		// 			if(t2 < 0)
		// 			{
		// 				if(t1 < 0)
		// 				{
		// 					if (t2 < 0)
		// 					{
		// 						return false;
		// 					}
		// 					else
		// 					{
		// 						t = t2;
		// 						return true;
		// 					}
		// 				}
		// 				else
		// 				{
		// 					t = t1;
		// 					return true;
		// 				}
		// 			}
		// 			else
		// 			{
		// 				t = t2;
		// 				return true;
		// 			}
		// 		}
	}
}

__inline bool RayPlaneIntersection(const glm::vec3 &n, const glm::vec3 &p0, const Ray &ray, float &t)
{
	// assuming vectors are all normalized
	float denom = glm::dot(n, ray.direction);
	if (denom > 1e-6) {
		glm::vec3 p0l0 = p0 - ray.origin;
		t = glm::dot(p0l0, n) / denom;
		return (t >= 0);
	}

	return false;
}

struct OctreeNode
{
	bool isLeafNode;
	glm::vec3 minCoordinates;
	glm::vec3 maxCoordinates;

	std::vector<Sphere*> spheres;
	OctreeNode* childNodes;


	OctreeNode(glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates,const std::vector<Sphere*> &spheres, bool isLeafNode)
	{
		this->minCoordinates = minCoordinates;
		this->maxCoordinates = maxCoordinates;
		this->isLeafNode = isLeafNode;
		this->spheres = spheres;

		this->childCount = 0;
		childNodes = new OctreeNode[8];
	}

	~OctreeNode()
	{
		delete []childNodes;
	}

	void AddChild(OctreeNode &node)
	{
		childNodes[childCount] = node;
		childCount++;
	}

private:
	int childCount;
	OctreeNode()
	{
		childCount = 0;
		childNodes = nullptr;
	}
};

struct ChildPoint
{
	int childIndex;
	float point;
};

__inline bool ChildPointComparisonFunc(const ChildPoint &a, const ChildPoint &b)
{
	return a.point < b.point;
}

class Octree
{
	int maxPrimitivePerLeaf;
	int maxLevel;
	OctreeNode *root;

	void FindBoundingBoxCoordinates(const std::vector<Sphere*> &spheres, glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates);
	void BuildChilds(OctreeNode *node, int currentLevel);
	void CreateChild(OctreeNode *node, glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates, int currentLevel);
	void FindChildSpheres(const std::vector<Sphere*> &parentSpheres, std::vector<Sphere*> &childSpheres, glm::vec3 &minCoordinates, glm::vec3 &maxCoordinates);

	bool CheckRayBoxIntersection(const Ray &ray, const glm::vec3 &boxMin, const glm::vec3 &boxMax, float &outTMin)
	{
		float tmin = -FLT_MAX, tmax = FLT_MAX;

		if (ray.direction.x != 0.0) {
			float tx1 = (boxMin.x - ray.origin.x) / ray.direction.x;
			float tx2 = (boxMax.x - ray.origin.x) / ray.direction.x;

			tmin = glm::max(tmin, glm::min(tx1, tx2));
			tmax = glm::min(tmax, glm::max(tx1, tx2));
		}

		if (ray.direction.y != 0.0) {
			float ty1 = (boxMin.y - ray.origin.y) / ray.direction.y;
			float ty2 = (boxMax.y - ray.origin.y) / ray.direction.y;

			tmin = glm::max(tmin, glm::min(ty1, ty2));
			tmax = glm::min(tmax, glm::max(ty1, ty2));
		}

		if (ray.direction.z != 0.0) {
			float tz1 = (boxMin.z - ray.origin.z) / ray.direction.z;
			float tz2 = (boxMax.z - ray.origin.z) / ray.direction.z;

			tmin = glm::max(tmin, glm::min(tz1, tz2));
			tmax = glm::min(tmax, glm::max(tz1, tz2));
		}
		outTMin = tmin;
		return tmax >= tmin;
	}
	bool FindClosestIntersectionOfNode(OctreeNode *node, const Ray &ray, IntersectionPoint &intersectionPoint, float& t)
	{
		if (node->isLeafNode)
		{
			float minT = FLT_MAX;
			int minIndex = -1;

			for (int i = 0; i < node->spheres.size(); i++)
			{
				if (RaySphereIntersection(*(node->spheres[i]), ray, t))
				{
					if (t < minT)
					{
						minIndex = i;
						minT = t;
					}
				}
			}

			if (minIndex != -1)
			{
				t = minT;
				intersectionPoint.position = ray.origin + ray.direction * t;
				intersectionPoint.color = node->spheres[minIndex]->color;
				intersectionPoint.normal = glm::normalize(intersectionPoint.position - node->spheres[minIndex]->center);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			ChildPoint indexDistancePairs[8];
			int foundIntersection = 0;
			for (int i = 0; i < 8; i++)
			{
				float intT;
				if(CheckRayBoxIntersection(ray, node->childNodes[i].minCoordinates, node->childNodes[i].maxCoordinates, intT))
				{
					indexDistancePairs[foundIntersection].point = intT;
					indexDistancePairs[foundIntersection].childIndex = i;
					foundIntersection++;
				}
			}
			if (foundIntersection > 1)
			{
				int a = 5;
			}
			std::sort(indexDistancePairs, indexDistancePairs + foundIntersection, ChildPointComparisonFunc);
			for (int i = 0; i < foundIntersection; i++)
			{
				const ChildPoint &curNodeInfo = indexDistancePairs[i];
				if(FindClosestIntersectionOfNode(&node->childNodes[curNodeInfo.childIndex], ray, intersectionPoint, t))
				{
					return true;
				}
			}

			return false;
		}
	}
public:
	Octree(int maxPrimitivePerLeaf, int maxLevel)
	{
		this->maxPrimitivePerLeaf = maxPrimitivePerLeaf;
		this->maxLevel = maxLevel;
		root = nullptr;
	}
	OctreeNode *GetRootNode()
	{
		return root;
	}
	void Build(const std::vector<Sphere*> &spheres);
	bool FindClosestIntersection(const Ray &ray, IntersectionPoint &intersectionPoint, float& t)
	{
		return FindClosestIntersectionOfNode(root, ray, intersectionPoint, t);
	}

	void Clear()
	{
		delete root;
	}
};