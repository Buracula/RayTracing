#pragma once

#define USE_OCTREE

#include <d3d11.h>
#include <glm\glm.hpp>
#include <vector>
#include "Sphere.h"
#include "Octree.h"



class RayTracer
{	
public:	//TODO public for debugging
	std::vector<Sphere*> mSpheres;
	Octree octree;
private:
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

public:
	RayTracer();

	void SetCameraParams(const glm::vec3 camPos, const glm::vec3 lookAt, const glm::vec3 upDir, int width, int height, float verticalFov);
	void AddSphere(Sphere *sphere)
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
	void Clear()
	{
		octree.Clear();
		for (int i_sphere = 0; i_sphere < mSpheres.size(); i_sphere++)
		{
			delete mSpheres[i_sphere];
		}
		mSpheres.clear();
		mLights.clear();
	}

	void CreateGpuBuffers(ID3D11Device *device, ID3D11ShaderResourceView **octreeBufferSRV, ID3D11ShaderResourceView **lightBufferSRV, ID3D11ShaderResourceView **sphereBufferSRV,
		ID3D11Buffer **constantBuffer);
};
