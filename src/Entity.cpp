#include "Entity.h"
#include <math.h>
using namespace DirectX;


Microsoft::WRL::ComPtr<ID3D12Resource> Entity::vertexBuffer;
Microsoft::WRL::ComPtr<ID3D12Resource> Entity::indexBuffer;

D3D12_VERTEX_BUFFER_VIEW Entity::vertexBufferView = {};
D3D12_INDEX_BUFFER_VIEW Entity::indexBufferView = {};



Entity::Entity(DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale, float rotation)
	:
	translation(origin), scale(scale), rotation(rotation), dPos(0,0), dScale(0,0), dRotation(0)
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

void Entity::UpdateDisplacementVectors(DirectX::XMFLOAT2 dPos, DirectX::XMFLOAT2 dScale, float dRotation)
{
	this->dPos.x += dPos.x;
	this->dPos.y += dPos.y;

	this->dScale.x += dScale.x;
	this->dScale.y += dScale.y;

	this->dRotation += dRotation;
}

void Entity::UpdatePosition()
{
	translation.x += dPos.x;
	translation.y += dPos.y;
	scale.x += dScale.x;
	scale.y += dScale.y;
	rotation += dRotation;

	dPos = { 0, 0 };
	dScale = { 0,0 };
	dRotation = 0;


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

	if (dPos.y < 0)
	{
		int x = 2;
	}
	float px_1 = translation.x + dPos.x - scale.x ;
	float px_2 = translation.x + dPos.x + scale.x ;

	float ex_1 = entity.translation.x + entity.dPos.x - entity.scale.x ;

	if (px_1 > ex_1)
	{
		swap(px_1, ex_1);
		px_2 = entity.translation.x + entity.dPos.x + entity.scale.x;
	}

	float py_1 = translation.y+ dPos.y - scale.y;
	float py_2 = translation.y+ dPos.y + scale.y;

	float ey_1 = entity.translation.y + entity.dPos.y - entity.scale.y ;


	if (py_1 > ey_1)
	{
		swap(py_1, ey_1);
		py_2 = entity.translation.y + entity.dPos.y + entity.scale.y;
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


float euclideanDistance(XMFLOAT2&& l, XMFLOAT2& r)
{
	return  sqrtf( powf(l.x - r.x,2) + powf(l.y - r.y, 2));
}

void Entity::ResolveCollision(Entity& staticEntity)
{

	if (dPos.x == 0 && dPos.y == 0)
	{
		return;
	}
	float expanded_x = staticEntity.scale.x + scale.x;
	float tx_1 = ((staticEntity.translation.x - expanded_x) - translation.x) / dPos.x;
	float tx_2 = ((staticEntity.translation.x + expanded_x) - translation.x) / dPos.x;

	if (tx_1 > tx_2)
	{
		swap(tx_1, tx_2);
	}

	float expanded_y = staticEntity.scale.y + scale.y;
	float ty_1 = ((staticEntity.translation.y - expanded_y) - translation.y) / dPos.y;
	float ty_2 = ((staticEntity.translation.y + expanded_y) - translation.y) / dPos.y;

	if (ty_1 > ty_2)
	{
		swap(ty_1, ty_2);
	}
	
	float t_hit_near = max(tx_1, ty_1);

	XMFLOAT2 contact_normal;
	if (tx_1 > ty_1)
		if (1.0f/dPos.x < 0)
			contact_normal = { 1, 0 };
		else
			contact_normal = { -1, 0 };
	else if (tx_1 < ty_1)
		if (1.0f / dPos.y < 0)
			contact_normal = { 0, 1 };
		else
			contact_normal = { 0, -1 };

	dPos = { dPos.x + contact_normal.x * abs(dPos.x) * (1 - t_hit_near) ,dPos.y + contact_normal.y * abs(dPos.y) * (1 - t_hit_near)};

}
