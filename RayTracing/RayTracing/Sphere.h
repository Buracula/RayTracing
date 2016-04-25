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
		if (boundingBoxMin.x > maxBoundary.x || boundingBoxMax.x < minBoundary.x)
		{
			return false;
		}
		if (boundingBoxMin.y > maxBoundary.y || boundingBoxMax.y < minBoundary.y)
		{
			return false;
		}
		if (boundingBoxMin.z > maxBoundary.z || boundingBoxMax.z < minBoundary.z)
		{
			return false;
		}
		return true;
	}
};