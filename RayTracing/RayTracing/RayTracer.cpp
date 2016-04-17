#include "RayTracer.h"
#include <assert.h>


void RayTracer::Trace(const glm::vec3 &ray, float *output)
{

}

RayTracer::RayTracer()
{
	mRenderTarget = nullptr;
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
	for (int y = 0; y < mHeight; y++)
	{
		for (int x = 0; x < mWidth; x++)
		{
			glm::vec2 ssPos(x, y);
			ssPos = ssPos / dimension;
			ssPos = ssPos * _2 - minus1;

			glm::vec3 pixelWorldPos = nearPlaneCenter + mNearPlaneUp * ssPos.y + mNearPlaneSide * ssPos.x;
			glm::vec3 rayDir = glm::normalize(pixelWorldPos - mCameraPos);
		}
	}
}
