#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include "Rendering/DeviceResources.h"
#include <wrl.h>
#define INDEX_COUNT 6


class Entity
{
public:
	Entity(DeviceResources* device, DirectX::XMFLOAT2 origin,
						DirectX::XMFLOAT2 scale = { 1,1 }, float rotation = 0);
	void InitDrawingResources(DeviceResources* device);
	void UpdateDisplacementVectors(DirectX::XMFLOAT2 dPos, DirectX::XMFLOAT2 dScale, float dRotation);
	void UpdatePosition();
	D3D12_GPU_VIRTUAL_ADDRESS getConstantBufferVirtualAddress();
	D3D12_VERTEX_BUFFER_VIEW* getVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW* getIndexBufferView();
	bool IsColliding(Entity& entity);
	void UpdateColor(bool isColliding);
	void ResolveCollision(Entity& staticEntity);

private:
	DirectX::XMFLOAT2 translation;
	DirectX::XMFLOAT2 scale;

	DirectX::XMFLOAT2 dPos;
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

