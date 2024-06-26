#include "Camera.h"

Camera::Camera(DeviceResources* device, XMFLOAT2 transitionCoords)
	:
	cameraCenter(0,0), scalling(1,1), invAspectRatio(1.0f/device->AspectRatio()), transitionCoords(transitionCoords)
{
	this->transitionCoords.y *= 1.0f/device->AspectRatio();

	int size = ceil(sizeof(XMFLOAT4X4) * 2.0 / 256) * 256;

	device->CreateUploadBuffer(&CBuffer, size);
	D3D12_RANGE readRange = { 0,0 };
	CBuffer->Map(0, &readRange, &cbufferMap);

	UpdateBuffer();
}

void Camera::UpdateCamera(Entity& entity, float dt)
{
	PhysicalDescriptor entityDescriptor = entity.getEntityDescriptor();
	XMFLOAT2 offset = { 0,0 };
	constexpr float stopRange = 0.4;
	constexpr float displacementPercent = 0.1;
	XMFLOAT2 screenBox = { transitionCoords.x * (1 / scalling.x), transitionCoords.y * (1 / scalling.y) };
	if ((entityDescriptor.center.x > cameraCenter.x  &&  entityDescriptor.center.x  < cameraCenter.x + stopRange * screenBox.x)
		||
		(entityDescriptor.center.x  < cameraCenter.x && entityDescriptor.center.x > cameraCenter.x - stopRange * screenBox.x)
		)
	{
		centerRight = false;
		centerLeft = false;
	}


	if (entityDescriptor.center.x > cameraCenter.x + screenBox.x || centerRight)
	{
		centerRight = true;
		offset.x += abs(entityDescriptor.center.x - cameraCenter.x + stopRange * screenBox.x) * dt * 2;
	}
	else if (entityDescriptor.center.x < cameraCenter.x - screenBox.x || centerLeft)
	{
		centerLeft = true;
		offset.x -= abs(entityDescriptor.center.x - cameraCenter.x - stopRange * screenBox.x) * dt * 2;
	}


	if ((entityDescriptor.center.y > cameraCenter.y && entityDescriptor.center.y < cameraCenter.y + stopRange * screenBox.y)
		||
		(entityDescriptor.center.y  < cameraCenter.y && entityDescriptor.center.y > cameraCenter.y - stopRange * screenBox.y)
		)
	{
		centerTop = false;
		centerBottom = false;
	}


	if (entityDescriptor.center.y > cameraCenter.y + screenBox.y || centerTop)
	{
		centerTop = true;
		offset.y += abs(entityDescriptor.center.y - cameraCenter.y + stopRange * screenBox.y) * dt * 2;
	}
	else if (entityDescriptor.center.y < cameraCenter.y - screenBox.y || centerBottom)
	{
		centerBottom = true;
		offset.y -= abs(entityDescriptor.center.y - cameraCenter.y - stopRange * screenBox.y) * dt * 2;
	}

	if (offset.x != 0 || offset.y != 0)
	{
		cameraCenter.x += offset.x;
		cameraCenter.y += offset.y;
		UpdateBuffer();
	}
}

void Camera::ZoomUpdate(float dScale, float low_cap, float high_cap)
{
	if (dScale != 0)
	{
		scalling.x += dScale;
		scalling.y += dScale;
		
		scalling.x = scalling.x < low_cap ? low_cap : scalling.x > high_cap ? high_cap : scalling.x;
		scalling.y = scalling.y < low_cap ? low_cap : scalling.y > high_cap ? high_cap : scalling.y;

		UpdateBuffer();
	}
}

XMFLOAT2 Camera::getCameraCenter()
{
	return cameraCenter;
}

XMFLOAT2 Camera::TransformCoordsToWorldCoords(XMFLOAT2 deviceCoords)
{
	return{ deviceCoords.x/ (invAspectRatio * scalling.x) + cameraCenter.x,   deviceCoords.y / scalling.y + cameraCenter.y };
}

XMFLOAT2 Camera::TransformToCameraCoords(XMFLOAT2 pos)
{
	return{ (pos.x - cameraCenter.x) * (invAspectRatio * scalling.x),   (pos.y - cameraCenter.y) * (scalling.y) };
}

XMFLOAT2 Camera::TransformCoordsToWorldCoordsWithRespectToEntity(XMFLOAT2 deviceCoords, Entity* entity)
{
	PhysicalDescriptor entityWorldCoord = entity->getEntityDescriptor();
	return{ deviceCoords.x / (invAspectRatio * scalling.x) + entityWorldCoord.center.x,   deviceCoords.y / (scalling.y) + entityWorldCoord.center.y };
}

D3D12_GPU_VIRTUAL_ADDRESS Camera::getWorldTransformAddress()
{
	return CBuffer->GetGPUVirtualAddress();
}

void Camera::UpdateBuffer()
{
	XMFLOAT2 scallingTransform = scalling;
	scallingTransform.x *= invAspectRatio;
	XMFLOAT2 cameraTranslation = { -cameraCenter.x, -cameraCenter.y };

	XMMATRIX transformation = XMMatrixTranslation(cameraTranslation.x, cameraTranslation.y, 0)* //camera translation
							  XMMatrixScaling(scalling.x, scalling.y, 0) * // zoom in-out
							  XMMatrixScaling(invAspectRatio, 1, 0); // perspective correction

	transformation = XMMatrixTranspose(transformation);
	XMFLOAT4X4 transformationMatrix;
	XMStoreFloat4x4(&transformationMatrix, transformation);
	memcpy(cbufferMap, &transformationMatrix, sizeof(XMFLOAT4X4));
}
