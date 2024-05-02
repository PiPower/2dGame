#include "Entity.h"

using namespace DirectX;


Microsoft::WRL::ComPtr<ID3D12Resource> Entity::vertexBuffer;
Microsoft::WRL::ComPtr<ID3D12Resource> Entity::indexBuffer;

D3D12_VERTEX_BUFFER_VIEW Entity::vertexBufferView = {};
D3D12_INDEX_BUFFER_VIEW Entity::indexBufferView = {};



Entity::Entity(DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale, float rotation)
	:
	translation(origin), scale(scale), rotation(rotation)
{
	if (vertexBuffer.Get() == nullptr)
	{
		InitDrawingResources(device);
	}
	XMVECTOR scallingVec = XMLoadFloat2(&scale);
	XMVECTOR rotationOriginVec = XMVectorZero();
	XMVECTOR translationVec = XMLoadFloat2(&translation);

	XMMATRIX transformation = XMMatrixAffineTransformation2D(scallingVec, rotationOriginVec, rotation, translationVec);
	transformation = XMMatrixTranspose(transformation);
	XMFLOAT4X4 transformationMatrix;
	XMStoreFloat4x4(&transformationMatrix, transformation);
	device->CreateUploadBuffer(&constantBuffer, 256);

	D3D12_RANGE range{ 0,0 };
	constantBuffer->Map(0, &range, (void**)&constantBufferMap);
	memcpy(constantBufferMap, &transformationMatrix, sizeof(XMFLOAT4X4));
}

void Entity::InitDrawingResources(DeviceResources* device)
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

void Entity::UpdatePosition(DirectX::XMFLOAT2 dPos, DirectX::XMFLOAT2 dScale, float dRotation)
{
	translation.x += dPos.x;
	translation.y += dPos.y;

	scale.x += dScale.x;
	scale.y += dScale.y;

	rotation += dRotation;

	XMVECTOR scallingVec = XMLoadFloat2(&scale);
	XMVECTOR rotationOriginVec = XMVectorZero();
	XMVECTOR translationVec = XMLoadFloat2(&translation);

	XMMATRIX transformation = XMMatrixAffineTransformation2D(scallingVec, rotationOriginVec, rotation, translationVec);
	transformation = XMMatrixTranspose(transformation);
	XMFLOAT4X4 transformationMatrix;
	XMStoreFloat4x4(&transformationMatrix, transformation);
	memcpy(constantBufferMap, &transformationMatrix, sizeof(XMFLOAT4X4));
}

D3D12_GPU_VIRTUAL_ADDRESS Entity::getConstantBufferVirtualAddress()
{
	return constantBuffer->GetGPUVirtualAddress();
}

D3D12_VERTEX_BUFFER_VIEW* Entity::getVertexBufferView()
{
	return &vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW* Entity::getIndexBufferView()
{
	return &indexBufferView;
}

bool Entity::IsColliding(Entity& entity)
{

	float px_1 = translation.x - scale.x ;
	float px_2 = translation.x + scale.x ;

	float ex_1 = entity.translation.x - entity.scale.x ;

	if (px_1 > ex_1)
	{
		swap(px_1, ex_1);
		px_2 = entity.translation.x + entity.scale.x;
	}

	float py_1 = translation.y - scale.y;
	float py_2 = translation.y + scale.y;

	float ey_1 = entity.translation.y - entity.scale.y ;


	if (py_1 > ey_1)
	{
		swap(py_1, ey_1);
		py_2 = entity.translation.y + entity.scale.y;
	}

	if (  ex_1 < px_2 && ey_1 < py_2 )
	{
		return true;
	}

	return false;
}

void Entity::UpdateColor(bool isColliding)
{
	if (isColliding)
	{
		float col[] = { 0.3,0.5, 0.6, 1 };
		memcpy((char*)constantBufferMap + sizeof(XMFLOAT4X4), col, sizeof(float) * 4);
	}
	else {
		float col[] = { 1, 1, 1, 1 };
		memcpy((char*)constantBufferMap + sizeof(XMFLOAT4X4), col, sizeof(float) * 4);
	}
}
