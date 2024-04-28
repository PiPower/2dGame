#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include "Rendering/DeviceResources.h"
#include <wrl.h>
#define INDEX_COUNT 6


class GraphicalObject
{
public:
	GraphicalObject(DeviceResources* device, DirectX::XMFLOAT2 center, DirectX::XMFLOAT2 scale = { 1,1 },
				DirectX::XMFLOAT2 offset = { 0,0 }, DirectX::XMFLOAT2 rotation = { 0,0 });
	void initDrawingResources(DeviceResources* device);
	D3D12_VERTEX_BUFFER_VIEW* getVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW* getIndexBufferView();

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer;
	DirectX::XMFLOAT4X4 transformationMatrix;

	static Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	static Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;

	static D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	static D3D12_INDEX_BUFFER_VIEW indexBufferView;
};

