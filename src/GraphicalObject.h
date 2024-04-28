#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include "Rendering/DeviceResources.h"
#include <wrl.h>
#define INDEX_COUNT 6


class GraphicalObject
{
public:
	GraphicalObject(DeviceResources* device, DirectX::XMFLOAT2 origin, 
						DirectX::XMFLOAT2 scale = { 1,1 }, float rotation = 0);
	void initDrawingResources(DeviceResources* device);
	D3D12_GPU_VIRTUAL_ADDRESS getConstantBufferVirtualAddress();
	D3D12_VERTEX_BUFFER_VIEW* getVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW* getIndexBufferView();

private:
	UINT* constantBufferMap;
	Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer;

	static Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	static Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;

	static D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	static D3D12_INDEX_BUFFER_VIEW indexBufferView;
};

