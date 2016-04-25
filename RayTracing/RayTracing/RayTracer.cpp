#include "RayTracer.h"
#include <assert.h>

RayTracer::RayTracer()
	:octree(3, 4)
{
	mRenderTarget = nullptr;
}


void RayTracer::Trace(const Ray &ray, glm::vec4 &outputColor)
{
	IntersectionPoint intersectionPoint;
	float t;
	if (FindClosestIntersection(ray, intersectionPoint, t))
	{
		outputColor = Shade(intersectionPoint, ray);
	}
	else
	{
		outputColor = glm::vec4(glm::vec3(0.5), 1.0f);
	}

}

glm::vec4 RayTracer::Shade(const IntersectionPoint &intersectionPoint, const Ray &ray)
{
	glm::vec4 output(0);
	//TODO ambient?
	Ray shadowRay;

	for (int i = 0;  i < mLights.size(); i++)
	{		
		CalculateShadowRay(intersectionPoint, ray, mLights[i], shadowRay);

		if(!InShadow(shadowRay, mLights[i]))
		{
			output += PhongIllumination(intersectionPoint, ray, mLights[i]);
		}
	}

	return output;
}

void RayTracer::CalculateShadowRay(const IntersectionPoint &intersectionPoint, const Ray &ray, const Light &light, Ray &shadowRay)
{
	shadowRay.direction = glm::normalize(light.position - intersectionPoint.position);
	shadowRay.origin = intersectionPoint.position + glm::vec3(0.005f) * intersectionPoint.normal;
}

bool RayTracer::InShadow(const Ray &shadowRay, const Light &light)
{
	IntersectionPoint intersectionPoint;
	float maxT = glm::length(light.position - shadowRay.origin);
	float intersectionT;
	if(FindClosestIntersection(shadowRay, intersectionPoint, intersectionT) && intersectionT < maxT)
	{
		return true;
	}

	return false;
}

glm::vec4 RayTracer::PhongIllumination(const IntersectionPoint &intersectionPoint, const Ray &ray, const Light &light)
{
	float specularPower = 40.0f;
	glm::vec4 totalColor;
	const glm::vec3 &surfaceNormal = intersectionPoint.normal;
	glm::vec3 surfaceToLight = light.position - intersectionPoint.position;
	glm::vec3 viewDir = glm::normalize(intersectionPoint.position - mCameraPos);
	float surfaceToLightLen = glm::length(surfaceToLight);
	surfaceToLight /= surfaceToLightLen;
	float NoL = glm::clamp(glm::dot(surfaceNormal, surfaceToLight), 0.0f, 1.0f);

	totalColor = intersectionPoint.color * light.color * NoL / (surfaceToLightLen * surfaceToLightLen);
	glm::vec3 h = glm::normalize(surfaceToLight + viewDir);
	float NoH = glm::clamp(glm::dot(surfaceNormal, h), 0.0f, 1.0f);
	totalColor += glm::pow(NoH, specularPower) * intersectionPoint.color * light.color / (surfaceToLightLen * surfaceToLightLen);
	return totalColor;
}

bool RayTracer::FindClosestIntersection(const Ray &ray, IntersectionPoint &intersectionPoint, float& t)
{
#ifdef USE_OCTREE
	float sceneT;
	float groundT;
	bool intersectionWithScene = octree.FindClosestIntersection(ray, intersectionPoint, sceneT);
	bool intersectionWithGround = RayPlaneIntersection(glm::vec3(0, 0, -1), glm::vec3(0,0,-10), ray, groundT);
	if (intersectionWithScene || intersectionWithGround)
	{
		if (intersectionWithScene && intersectionWithGround)
		{
			if (groundT < sceneT)
			{
				intersectionPoint.normal = glm::vec3(0, 0, 1);
				intersectionPoint.position = ray.origin + ray.direction * groundT;
				intersectionPoint.color = glm::vec4(0.6, 0.6, 0.6,1);
				t = groundT;
			}
			else
			{
				t = sceneT;
			}
		}
		else
		{
			if (intersectionWithScene)
			{
				t = sceneT;
			}
			else
			{
				intersectionPoint.normal = glm::vec3(0, 0, 1);
				intersectionPoint.position = ray.origin + ray.direction * groundT;
				intersectionPoint.color = glm::vec4(0.6, 0.6, 0.6, 1);
				t = groundT;
			}
		}
		return true;
	}
	return false;
#else
	float minT = FLT_MAX;
	int minIndex = -1;

	for (int i = 0; i < mSpheres.size(); i++)
	{
		if(RaySphereIntersection((*mSpheres[i]), ray, t))
		{
			if(t < minT)
			{
				minIndex = i;
				minT = t;
			}
		}
	}

	if(minIndex != -1)
	{
		t = minT;
		intersectionPoint.position = ray.origin + ray.direction * t;
		intersectionPoint.color = mSpheres[minIndex]->color;
		intersectionPoint.normal = glm::normalize(intersectionPoint.position - mSpheres[minIndex]->center);
		return true;
	}
	else
	{
		return false;
	}
#endif
}


void RayTracer::SetCameraParams(const glm::vec3 camPos, const glm::vec3 lookAt, const glm::vec3 upDir, int width, int height, float verticalFov)
{
	mCameraPos = camPos;
	mAspectRatio = (float)width / height;
	mVerticalFov = verticalFov;
	glm::vec3 camDir = lookAt - mCameraPos;
	assert(glm::length(camDir) != 0);
	mCameraDir = glm::normalize(camDir);
	glm::vec3 upDirNormalized = glm::normalize(upDir);
	float nearPlaneHalfHeight = glm::tan(glm::radians(verticalFov));
	float nearPlaneHalfWidth = nearPlaneHalfHeight * mAspectRatio;

	glm::vec3 cameraSideNormalized = glm::normalize(glm::cross(mCameraDir, upDirNormalized));
	glm::vec3 cameraUpNormalized = glm::normalize(glm::cross(cameraSideNormalized, mCameraDir));

	mNearPlaneSide = cameraSideNormalized * nearPlaneHalfWidth;
	mNearPlaneUp = cameraUpNormalized * nearPlaneHalfHeight;

	mWidth = width;
	mHeight = height;

	delete[]mRenderTarget;

	mRenderTarget = new float[mWidth * mHeight * 4]; //RGBA
	memset(mRenderTarget, 0x00, sizeof(float) * mWidth * mHeight * 4);

}

void RayTracer::Update()
{
	octree.Build(mSpheres);

	glm::vec2 dimension(mWidth, mHeight);
	glm::vec2 _2(2.0f);
	glm::vec2 minus1(-1.0f);
	glm::vec3 nearPlaneCenter = mCameraPos + mCameraDir;
	float *targetPtr = mRenderTarget;
	for (int y = 0; y < mHeight; y++)
	{
		for (int x = 0; x < mWidth; x++)
		{
			if(x == 1042 && y == (343 - 31))
			{
				int a = 5;
			}
			glm::vec2 ssPos(x, y);
			ssPos = ssPos / dimension;
			ssPos = ssPos * _2 + minus1;

			glm::vec3 pixelWorldPos = nearPlaneCenter + mNearPlaneUp * -ssPos.y + mNearPlaneSide * ssPos.x;
			glm::vec3 rayDir = glm::normalize(pixelWorldPos - mCameraPos);
			Ray ray;
			ray.direction = rayDir;
			ray.origin = mCameraPos;
			glm::vec4 oc;
			Trace(ray, oc);
			targetPtr[0] = oc.r;
			targetPtr[1] = oc.g;
			targetPtr[2] = oc.b;
			targetPtr[3] = oc.w;
			
			targetPtr += 4;
		}
	}
}






