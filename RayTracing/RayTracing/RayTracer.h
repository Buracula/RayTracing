#pragma once

#include <glm\glm.hpp>
#include <vector>

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


struct Sphere
{
	glm::vec3 center;
	glm::vec4 color;
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
	glm::vec4 color;

	Light()
	{
		position = glm::vec3(0);
		color = glm::vec4(1);
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

	void Trace(const Ray &ray, glm::vec4 &outputColor);
	bool FindClosestIntersection(const Ray &ray, IntersectionPoint &intersectionPoint, float& t);
	glm::vec4 Shade(const IntersectionPoint &intersectionPoint, const Ray &ray);
	void CalculateShadowRay(const IntersectionPoint &intersectionPoint, const Ray &ray, const Light &light, Ray &shadowRay);
	bool InShadow(const Ray &shadowRay, const Light &light);
	glm::vec4 PhongIllumination(const IntersectionPoint &intersectionPoint, const Ray &ray, const Light &light);
	bool RaySphereIntersection(const Sphere &sphere, const Ray &ray, float &t);

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
	const float *GetRenderTargetBuffer()
	{
		return mRenderTarget;
	}
	void Update();
};
