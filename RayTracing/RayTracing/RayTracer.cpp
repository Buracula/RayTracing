#include "RayTracer.h"
#include <assert.h>

RayTracer::RayTracer()
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
			output = output + PhongIllumination(intersectionPoint, ray, mLights[i]);
		}
	}

	return output;
}

void RayTracer::CalculateShadowRay(const IntersectionPoint &intersectionPoint, const Ray &ray, const Light &light, Ray &shadowRay)
{
	shadowRay.direction = glm::normalize(light.position - shadowRay.origin);
	shadowRay.origin = intersectionPoint.position + glm::vec3(0.0001f) * shadowRay.direction;	
}

bool RayTracer::InShadow(const Ray &shadowRay, const Light &light)
{
	IntersectionPoint intersectionPoint;
	float maxT = glm::length(light.position - shadowRay.origin);
	float intersectionT;
	for (int i = 0; i < mSpheres.size(); i++)
	{
		if(FindClosestIntersection(shadowRay, intersectionPoint, intersectionT) && intersectionT < maxT)
		{
			return true;
		}
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
		intersectionPoint.position = ray.origin + ray.direction * t;
		intersectionPoint.color = mSpheres[minIndex]->color;
		intersectionPoint.normal = glm::normalize(intersectionPoint.position - mSpheres[minIndex]->center);
		t = minT;
		return true;
	}
	else
	{
		return false;
	}
}

bool RayTracer::RaySphereIntersection(const Sphere &sphere, const Ray &ray, float &t)
{
	glm::vec3 o_c = ray.origin - sphere.center;
	float dDotD = glm::dot(ray.direction, ray.direction);

	float delta = glm::pow(glm::dot(ray.direction, (o_c)), 2)  
		-dDotD *( glm::dot(o_c, o_c) - pow(sphere.radius, 2));

	if(delta < 0)
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

		if(minRoot < 0)
		{
			if(maxRoot < 0)
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

	glm::vec3 cameraSideNormalized = glm::cross(mCameraDir, upDirNormalized);
	glm::vec3 cameraUpNormalized = glm::cross(cameraSideNormalized, mCameraDir);

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
	glm::vec2 dimension(mWidth, mHeight);
	glm::vec2 _2(2.0f);
	glm::vec2 minus1(-1.0f);
	glm::vec3 nearPlaneCenter = mCameraPos + mCameraDir;
	float *targetPtr = mRenderTarget;
	for (int y = 0; y < mHeight; y++)
	{
		for (int x = 0; x < mWidth; x++)
		{
			if(x == mWidth / 2 && y == mHeight/2)
			{
				int a = 5;
			}
			glm::vec2 ssPos(x, y);
			ssPos = ssPos / dimension;
			ssPos = ssPos * _2 + minus1;

			glm::vec3 pixelWorldPos = nearPlaneCenter + mNearPlaneUp * ssPos.y + mNearPlaneSide * ssPos.x;
			glm::vec3 rayDir = glm::normalize(pixelWorldPos - mCameraPos);
			Ray ray;
			ray.direction = rayDir;
			ray.origin = mCameraPos;
			glm::vec4 oc;
			Trace(ray, oc);
			if(ssPos.g > 0)
			{
				int a = 5;
			}
// 			int myindex = (y * mWidth + x) * 4;
// 
// 			mRenderTarget[(y * mWidth + x) * 4 + 0] = rand() / (float)RAND_MAX;
// 			mRenderTarget[(y * mWidth + x) * 4 + 1] = rand() / (float)RAND_MAX;
// 			mRenderTarget[(y * mWidth + x) * 4 + 2] = 0;
// 			mRenderTarget[(y * mWidth + x) * 4 + 3] = 0;

			targetPtr[0] = oc.r;
			targetPtr[1] = oc.g;
			targetPtr[2] = oc.b;
			targetPtr[3] = oc.w;
			
			targetPtr += 4;
		}
	}
}






