#include "GraphicalObject.h"

using namespace DirectX;


Microsoft::WRL::ComPtr<ID3D12Resource> GraphicalObject::vertexBuffer;
Microsoft::WRL::ComPtr<ID3D12Resource> GraphicalObject::indexBuffer;

D3D12_VERTEX_BUFFER_VIEW GraphicalObject::vertexBufferView = {};
D3D12_INDEX_BUFFER_VIEW GraphicalObject::indexBufferView = {};



GraphicalObject::GraphicalObject(DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale, float rotation)
{
	if (vertexBuffer.Get() == nullptr)
	{
		initDrawingResources(device);
	}
	XMVECTOR scalling = XMLoadFloat2(&scale);
	XMVECTOR rotationOrigin = XMVectorZero();
	XMVECTOR translation = XMLoadFloat2(&origin);

	XMMATRIX transformation = XMMatrixAffineTransformation2D(scalling, rotationOrigin, 0, translation);
	transformation = XMMatrixTranspose(transformation);
	XMFLOAT4X4 transformationMatrix;
	XMStoreFloat4x4(&transformationMatrix, transformation);
	device->CreateUploadBuffer(&constantBuffer, 256);

	D3D12_RANGE range{ 0,0 };
	constantBuffer->Map(0, &range, (void**)&constantBufferMap);
	memcpy(constantBufferMap, &transformationMatrix, sizeof(XMFLOAT4X4));
}

void GraphicalObject::initDrawingResources(DeviceResources* device)
{
	//2 float for pos, 2 floats for tex coord
	XMFLOAT4 vertecies[4] = { {-1, 1, 0, 0}, {1, 1, 1, 0}, {1, -1, 1, 1}, {-1,-1, 0, 1} };
	unsigned int indecies[6] = {0 , 1, 3, 3, 1, 2 };

	device->CreateUploadBuffer(&vertexBuffer, sizeof(XMFLOAT4) * 4);
	device->CreateUploadBuffer(&indexBuffer, sizeof(unsigned int) * 6);

	UINT* mapVB, * mapIB;
	D3D12_RANGE readRange{ 0,0 };
	vertexBuffer->Map(0, &readRange, (void**) & mapVB);
	memcpy(mapVB, vertecies, sizeof(XMFLOAT4) * 4);
	vertexBuffer->Unmap(0, nullptr);

	indexBuffer->Map(0, &readRange, (void**)&mapIB);
	memcpy(mapIB, indecies, sizeof(unsigned int) * 6);
	indexBuffer->Unmap(0, nullptr);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress() ;
	vertexBufferView.SizeInBytes = sizeof(XMFLOAT4) * 4;
	vertexBufferView.StrideInBytes = sizeof(XMFLOAT4);

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(unsigned int) * 6;
}

D3D12_GPU_VIRTUAL_ADDRESS GraphicalObject::getConstantBufferVirtualAddress()
{
	return constantBuffer->GetGPUVirtualAddress();
}

D3D12_VERTEX_BUFFER_VIEW* GraphicalObject::getVertexBufferView()
{
	return &vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW* GraphicalObject::getIndexBufferView()
{
	return &indexBufferView;
}
