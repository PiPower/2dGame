#pragma once
#include "../Rendering/DeviceResources.h"
#include "Entity.h"
class Camera
{
public:
	Camera(DeviceResources* device, XMFLOAT2 transitionCoords = {1, 1});
	void UpdateCamera(Entity& entity, float dt);
	void ZoomUpdate(float dScale, float low_cap = 0.4, float high_cap = 2);
	XMFLOAT2 getCameraCenter();
	XMFLOAT2 TransformCoordsToWorldCoords(XMFLOAT2 deviceCoords);
	XMFLOAT2 TransformToCameraCoords(XMFLOAT2 pos);
	XMFLOAT2 TransformCoordsToWorldCoordsWithRespectToEntity(XMFLOAT2 deviceCoords, Entity* entity);
	D3D12_GPU_VIRTUAL_ADDRESS getWorldTransformAddress();
private:
	void UpdateBuffer();
private:
	void* cbufferMap;
	ComPtr<ID3D12Resource> CBuffer;
	XMFLOAT2 cameraCenter;
	XMFLOAT2 scalling;
	XMFLOAT2 transitionCoords;
	float invAspectRatio;
	bool centerLeft;
	bool centerRight;
	bool centerTop;
	bool centerBottom;
};

