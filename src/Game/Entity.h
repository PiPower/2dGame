#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include "../Rendering/DeviceResources.h"
#include <wrl.h>
#define INDEX_COUNT 6

class Entity;

struct CollisionDescriptor
{
	bool collisionOccured;
	float t_hit;
	XMFLOAT2 surfaceNormal;
	Entity* collider;
};


struct PhysicalDescriptor
{
	XMFLOAT2 center;
	float width;
	float height;
	XMFLOAT2 velocity;
};

class Entity
{
public:
	Entity(DeviceResources* device, DirectX::XMFLOAT2 origin,
						DirectX::XMFLOAT2 scale = { 1,1 }, float rotation = 0);
	void InitDrawingResources(DeviceResources* device);
	void UpdateDisplacementVectors(DirectX::XMFLOAT2 dPos, DirectX::XMFLOAT2 dScale, float dRotation);
	void UpdatePosition(bool keepVelocity = false);
	D3D12_GPU_VIRTUAL_ADDRESS getConstantBufferVirtualAddress();
	D3D12_VERTEX_BUFFER_VIEW* getVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW* getIndexBufferView();
	CollisionDescriptor IsColliding(Entity& entity);
	PhysicalDescriptor getEntityDescriptor();
	void UpdateColor(XMFLOAT4 color);
	void ResolveCollision(CollisionDescriptor& desc);

protected:
	DirectX::XMFLOAT2 translation;
	DirectX::XMFLOAT2 scale;

	DirectX::XMFLOAT2 velocity;
	DirectX::XMFLOAT2 dScale;
	float dRotation;
	float rotation = 0;

	UINT* constantBufferMap;
	Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer;

	static Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	static Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;

	static D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	static D3D12_INDEX_BUFFER_VIEW indexBufferView;
};

