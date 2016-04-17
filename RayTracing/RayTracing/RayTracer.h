#pragma once

#include <glm\glm.hpp>
#include <vector>

struct Sphere
{
	glm::vec3 center;
	glm::vec3 color;
	float radius;

	Sphere()
	{
		center = glm::vec3(0);
		radius = 1;
	}
};

struct Light
{
	glm::vec3 position;
	glm::vec3 color;

	Light()
	{
		position = glm::vec3(0);
		color = glm::vec3(1);
	}
};

class RayTracer
{	
	std::vector<Sphere> mSpheres;
	std::vector<Light> mLights;
	glm::vec3 mCameraPos;
	glm::vec3 mCameraDir;
	glm::vec3 mNearPlaneSide;
	glm::vec3 mNearPlaneUp;
	int mWidth;
	int mHeight;
	float mVerticalFov;
	float mAspectRatio;
	float *mRenderTarget;

	void Trace(const glm::vec3 &ray, float *output);

public:
	RayTracer();

	void SetCameraParams(const glm::vec3 camPos, const glm::vec3 lookAt, const glm::vec3 upDir, int width, int height, float verticalFov);
	void AddSphere(const Sphere &sphere)
	{
		mSpheres.push_back(sphere);
	}
	void AddLight(const Light &light)
	{
		mLights.push_back(light);
	}
	void Update();
};
