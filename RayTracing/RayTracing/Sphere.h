#pragma once

#include <glm\glm.hpp>
#include <vector>

struct Sphere
{
	glm::vec3 center;
	glm::vec4 color;
	float radius;

	glm::vec3 boundingBoxMin;
	glm::vec3 boundingBoxMax;

	Sphere()
	{
		center = glm::vec3(0);
		radius = 1;

		CalculateBoundingBox();
	}

	Sphere(float radius, glm::vec3 center, glm::vec4 color)
	{
		this->center = center;
		this->radius = radius;
		this->color = color;

		CalculateBoundingBox();
	}

	void CalculateBoundingBox()
	{
		boundingBoxMin = center - radius;
		boundingBoxMax = center + radius;
	}

	bool IsInsideBoundaries(glm::vec3 minBoundary, glm::vec3 maxBoundary)
	{
		glm::vec3 boundingBoxVertices[8];
		boundingBoxVertices[0] = boundingBoxMin;
		boundingBoxVertices[1] = glm::vec3(boundingBoxMax.x, boundingBoxMin.y, boundingBoxMin.z);
		boundingBoxVertices[2] = glm::vec3(boundingBoxMin.x, boundingBoxMax.y, boundingBoxMin.z);
		boundingBoxVertices[3] = glm::vec3(boundingBoxMax.x, boundingBoxMax.y, boundingBoxMin.z);

		boundingBoxVertices[4] = glm::vec3(boundingBoxMin.x, boundingBoxMin.y, boundingBoxMax.z);
		boundingBoxVertices[5] = glm::vec3(boundingBoxMax.x, boundingBoxMin.y, boundingBoxMax.z);
		boundingBoxVertices[6] = glm::vec3(boundingBoxMin.x, boundingBoxMax.y, boundingBoxMax.z);
		boundingBoxVertices[7] = boundingBoxMax;

		for (int i = 0; i < 8; i++)
		{
			if(minBoundary.x <= boundingBoxVertices[i].x && minBoundary.y <= boundingBoxVertices[i].y && minBoundary.z <= boundingBoxVertices[i].z
				&& boundingBoxVertices[i].x <= maxBoundary.x && boundingBoxVertices[i].y <= maxBoundary.y && boundingBoxVertices[i].z <= maxBoundary.z)
			{
				return true;
			}
		}
		return false;
	}
};