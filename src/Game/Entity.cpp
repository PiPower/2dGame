#include "Entity.h"
#include <math.h>
#include <cmath>
#include <DirectXCollision.h>
#include <array>

struct Simplex {
private:
	std::array<XMFLOAT3, 4> m_points;
	int m_size;

public:
	Simplex()
		: m_size(0)
	{}

	Simplex& operator=(std::initializer_list<XMFLOAT3> list)
	{
		m_size = 0;
		for (XMFLOAT3 point : list)
			m_points[m_size++] = point;

		return *this;
	}

	void push_front(XMFLOAT3 point)
	{
		m_points = { point, m_points[0], m_points[1], m_points[2] };
		m_size = min(m_size + 1, 4);
	}

	XMFLOAT3& operator[](int i) { return m_points[i]; }
	size_t size() const { return m_size; }

	auto begin() const { return m_points.begin(); }
	auto end() const { return m_points.end() - (4 - m_size); }
};





using namespace DirectX;


Microsoft::WRL::ComPtr<ID3D12Resource> Entity::vertexBuffer;
Microsoft::WRL::ComPtr<ID3D12Resource> Entity::indexBuffer;

D3D12_VERTEX_BUFFER_VIEW Entity::vertexBufferView = {};
D3D12_INDEX_BUFFER_VIEW Entity::indexBufferView = {};



Entity::Entity(DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale, float rotation)
	:
	translation(origin), scale(scale), rotation(rotation), velocity(0,0), dScale(0,0), dRotation(0)
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

void Entity::UpdateDisplacementVectors(DirectX::XMFLOAT2 velocity, DirectX::XMFLOAT2 dScale, float dRotation)
{
	this->velocity.x += velocity.x;
	this->velocity.y += velocity.y;

	this->dScale.x += dScale.x;
	this->dScale.y += dScale.y;

	this->dRotation += dRotation;
}

void Entity::UpdatePosition(bool keepVelocity)
{
	translation.x += velocity.x;
	translation.y += velocity.y;
	scale.x += dScale.x;
	scale.y += dScale.y;
	rotation += dRotation;

	if (!keepVelocity)
	{
		velocity = { 0, 0 };
	}
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

bool Entity::isMoving()
{
	return velocity.x !=0 || velocity.y != 0;
}

CollisionDescriptor Entity::DynamicIntersection(Entity* entity)
{
	// GJK algorithm
	// 3rd dim is time 
	XMFLOAT3 parallelepipedCenter{ translation.x + 0.5f * velocity.x, translation.y + 0.5f * velocity.y, 0.5f };
	XMFLOAT3 parallelepipedCenterEntity{ entity->translation.x + 0.5f * entity->velocity.x, entity->translation.y + 0.5f * entity->velocity.y, 0.5f };

	// Simplex in 3 dims has 4 edges
	Simplex simplex;
	XMFLOAT3 A =  getSupport(entity, {1,0,0});
	simplex.push_front(A);
	int i = 1;
	while (true)
	{
		XMFLOAT3 dir = { -A.x, -A.y, -A.z };
		XMFLOAT3 newEdge = getSupport(entity, dir);
		float dotProd = newEdge.x * dir.x + newEdge.y * dir.y + newEdge.z * dir.z;
		if (dotProd <= 0)
		{
			return { false };
		}
		simplex.push_front(newEdge);
		if (nextSimplex(simplex, dir))
		{

			XMFLOAT2 center = translation;
			XMFLOAT2 enemyCenter = entity->translation;
			float diff_x = translation.x - enemyCenter.x;
			float diff_y = translation.y - enemyCenter.y;
			return { true , sqrtf(diff_x * diff_x + diff_y* diff_y)   , {0,0}, entity};
		}
	}
}

vector<XMFLOAT3> Entity::getParallelepipedVecs()
{
	vector<XMFLOAT3> centerEdgeVec;
	centerEdgeVec.resize(8);
	// 3rd dim is time 
	XMFLOAT3 parallelepipedCenter{ translation.x + 0.5f * velocity.x, translation.y + 0.5f * velocity.y, 0.5f };

	// front order: left top, right top, left bottom, right bottom
	centerEdgeVec[0] = { translation.x - scale.x - parallelepipedCenter.x, translation.y + scale.y - parallelepipedCenter.y, -0.5f };
	centerEdgeVec[1] = { translation.x + scale.x - parallelepipedCenter.x, translation.y + scale.y - parallelepipedCenter.y, -0.5f };
	centerEdgeVec[2] = { translation.x - scale.x - parallelepipedCenter.x, translation.y - scale.y - parallelepipedCenter.y, -0.5f};
	centerEdgeVec[3] = { translation.x + scale.x - parallelepipedCenter.x, translation.y - scale.y - parallelepipedCenter.y, -0.5f };

	// back order: left top, right top, left bottom, right bottom
	XMFLOAT3 backTranslation = { translation.x + velocity.x, translation.y + velocity.y, 0.5 };
	centerEdgeVec[4] = { backTranslation.x - scale.x - parallelepipedCenter.x, backTranslation.y + scale.y - parallelepipedCenter.y, 0.5f };
	centerEdgeVec[5] = { backTranslation.x + scale.x - parallelepipedCenter.x, backTranslation.y + scale.y - parallelepipedCenter.y, 0.5f };
	centerEdgeVec[6] = { backTranslation.x - scale.x - parallelepipedCenter.x, backTranslation.y - scale.y - parallelepipedCenter.y, 0.5f };
	centerEdgeVec[7] = { backTranslation.x + scale.x - parallelepipedCenter.x, backTranslation.y - scale.y - parallelepipedCenter.y, 0.5f };

	return centerEdgeVec;
}

bool Entity::line(Simplex& simplex, XMFLOAT3& dir)
{
	XMVECTOR a = XMLoadFloat3(&simplex[0]);
	XMVECTOR b = XMLoadFloat3(&simplex[1]);

	XMVECTOR ab = b - a;
	XMVECTOR ao = -a;

	XMVECTOR prod = XMVector3Dot(ab, ao);
	XMFLOAT3 prodBuff;

	XMStoreFloat3(&prodBuff, prod);

	if (prodBuff.x > 0)
	{
		XMStoreFloat3(
			&dir,
			XMVector3Cross(XMVector3Cross(ab, ao), ab)
		);
	}
	else
	{
		simplex = { simplex[0] };
		XMStoreFloat3(&dir, ao);
	}
	return false;
}

bool Entity::triangle(Simplex& simplex, XMFLOAT3& dir)
{
	XMVECTOR a = XMLoadFloat3(&simplex[0]);
	XMVECTOR b = XMLoadFloat3(&simplex[1]);
	XMVECTOR c = XMLoadFloat3(&simplex[2]);

	XMVECTOR ab = b - a;
	XMVECTOR ac = c - a;
	XMVECTOR ao = -a;

	XMVECTOR abc = XMVector3Cross(ab, ac);
	XMFLOAT3 prodBuff;

	XMStoreFloat3(&prodBuff,  XMVector2Dot(XMVector3Cross(abc, ac), ao) );
	if (prodBuff.x > 0)
	{
		XMStoreFloat3(&prodBuff, XMVector2Dot(ac, ao));
		if (prodBuff.x > 0)
		{
			simplex = { simplex[0], simplex[2] };
			XMStoreFloat3(&dir,XMVector3Cross(XMVector3Cross(ac, ao), ac) );
		}
		else
		{
			return line(simplex = { simplex[0], simplex[1]}, dir);
		}
	}
	else
	{
		XMStoreFloat3(&prodBuff, XMVector2Dot(XMVector3Cross(ab, abc), ao));
		if (prodBuff.x > 0)
		{
			return line(simplex = { simplex[0], simplex[1] }, dir);
		}

		else
		{
			XMStoreFloat3(&prodBuff, XMVector2Dot(abc, ao));
			if (prodBuff.x > 0)
			{
				XMStoreFloat3(&dir, abc);
			}

			else
			{
				simplex = {simplex[0], simplex[2], simplex[1]};
				XMStoreFloat3(&dir, -abc);
			}
		}
	}


	return false;
}

bool Entity::tetrahedron(Simplex& simplex, XMFLOAT3& dir)
{
	XMVECTOR a = XMLoadFloat3(&simplex[0]);
	XMVECTOR b = XMLoadFloat3(&simplex[1]);
	XMVECTOR c = XMLoadFloat3(&simplex[2]);
	XMVECTOR d = XMLoadFloat3(&simplex[3]);

	XMVECTOR ab = b - a;
	XMVECTOR ac = c - a;
	XMVECTOR ad = d - a;
	XMVECTOR ao = -a;

	XMVECTOR abc = XMVector3Cross(ab, ac);
	XMVECTOR acd = XMVector3Cross(ac, ad);
	XMVECTOR adb = XMVector3Cross(ad, ab);

	XMFLOAT3 prodBuff;


	XMStoreFloat3(&prodBuff, XMVector2Dot(abc, ao));
	if (prodBuff.x > 0)
	{
		return triangle(simplex = {simplex[0], simplex[1], simplex[2]}, dir);
	}

	XMStoreFloat3(&prodBuff, XMVector2Dot(acd, ao));
	if (prodBuff.x > 0)
	{
		return triangle(simplex = {simplex[0], simplex[2], simplex[3]}, dir);
	}

	XMStoreFloat3(&prodBuff, XMVector2Dot(adb, ao));
	if (prodBuff.x > 0)
	{
		return triangle(simplex = { simplex[0], simplex[3], simplex[1]}, dir);
	}

	return true;
}

bool Entity::nextSimplex(Simplex& simplex, XMFLOAT3& dir)
{

	switch (simplex.size())
	{
	case 2: return line(simplex, dir);
	case 3: return triangle(simplex, dir);
	case 4: return tetrahedron(simplex, dir);
	default:
		printf("unsupported number of vertecies for simples\n");
		exit(-1);
		break;
	}
	return false;
}

vector<XMFLOAT3> Entity::getParallelepipedEdges()
{
	vector<XMFLOAT3> centerEdges;
	centerEdges.resize(8);

	// front order: left top, right top, left bottom, right bottom
	centerEdges[0] = { translation.x - scale.x, translation.y + scale.y, 0.0f };
	centerEdges[1] = { translation.x + scale.x, translation.y + scale.y, 0.0f };
	centerEdges[2] = { translation.x - scale.x, translation.y - scale.y, 0.0f };
	centerEdges[3] = { translation.x + scale.x, translation.y - scale.y, 0.0f };

	// back order: left top, right top, left bottom, right bottom
	XMFLOAT3 backTranslation = { translation.x + velocity.x, translation.y + velocity.y, 1.0f };
	centerEdges[4] = { backTranslation.x - scale.x, backTranslation.y + scale.y, 1.0f };
	centerEdges[5] = { backTranslation.x + scale.x, backTranslation.y + scale.y, 1.0f };
	centerEdges[6] = { backTranslation.x - scale.x, backTranslation.y - scale.y, 1.0f };
	centerEdges[7] = { backTranslation.x + scale.x, backTranslation.y - scale.y, 1.0f };

	return centerEdges;
}

XMFLOAT3 Entity::getSupport(Entity* entity, XMFLOAT3 dir)
{

	vector<XMFLOAT3> centerEdgeVec = getParallelepipedEdges();
	vector<XMFLOAT3> centerEdgeVecEntity = entity->getParallelepipedEdges();

	XMFLOAT3 selfFurthestPoint = furthestPointinDir(centerEdgeVec, dir);
	XMFLOAT3 entityFurthestPoint = entity->furthestPointinDir(centerEdgeVecEntity, { -dir.x, -dir.y, -dir.z });
	return { selfFurthestPoint.x - entityFurthestPoint.x, selfFurthestPoint.y - entityFurthestPoint.y, selfFurthestPoint.z - entityFurthestPoint.z} ;
}

XMFLOAT3 Entity::furthestPointinDir(vector<XMFLOAT3> vecs, XMFLOAT3 dir)
{
	XMVECTOR dirVec = XMLoadFloat3(&dir);
	XMVECTOR pointVec = XMLoadFloat3(&vecs[0]);
	XMFLOAT3 dotBuff;
	XMStoreFloat3(&dotBuff, XMVector2Dot(dirVec, pointVec));
	XMFLOAT3 maxPoint = vecs[0];
	float maxProd = dotBuff.x;

	for (int i = 1; i < vecs.size(); i++)
	{
		pointVec = XMLoadFloat3(&vecs[i]);
		XMStoreFloat3(&dotBuff, XMVector2Dot(dirVec, pointVec));
		if (dotBuff.x > maxProd)
		{
			maxPoint = vecs[i];
			maxProd = dotBuff.x;
		}
	}



	return maxPoint;
}

CollisionDescriptor Entity::IsColliding(Entity* entity)
{

	if (velocity.x == 0 && velocity.y == 0)
	{
		return { false };
	}

	float expanded_x = entity->scale.x + scale.x;
	float tx_1 = ((entity->translation.x - expanded_x) - translation.x) / velocity.x;
	float tx_2 = ((entity->translation.x + expanded_x) - translation.x) / velocity.x;

	if (tx_1 > tx_2)
	{
		swap(tx_1, tx_2);
	}

	float expanded_y = entity->scale.y + scale.y;
	float ty_1 = ((entity->translation.y - expanded_y) - translation.y) / velocity.y;
	float ty_2 = ((entity->translation.y + expanded_y) - translation.y) / velocity.y;

	if (ty_1 > ty_2)
	{
		swap(ty_1, ty_2);
	}


	float t_hit_near = max(tx_1, ty_1);

	if (t_hit_near < 0 || t_hit_near > 1.0f) return { false };


	if (isnan(ty_2) || isnan(tx_2)) return { false };
	if (isnan(ty_1) || isnan(tx_1)) return { false };

	if (tx_1 > ty_2 || ty_1 > tx_2) return { false };
	if(min(tx_2, ty_2) < 0 ) return { false };

	XMFLOAT2 contact_normal;
	if (tx_1 >= ty_1)
		if (1.0f / velocity.x < 0)
			contact_normal = { 1, 0 };
		else
			contact_normal = { -1, 0 };
	else if (tx_1 < ty_1)
		if (1.0f / velocity.y < 0)
			contact_normal = { 0, 1 };
		else
			contact_normal = { 0, -1 };

	return { true, t_hit_near, contact_normal, entity };
}

PhysicalDescriptor Entity::getEntityDescriptor()
{
	PhysicalDescriptor out;
	out.center = translation;
	out.width = scale.x * 2;
	out.height = scale.y * 2;
	out.velocity = velocity;
	return out;
}

void Entity::UpdateColor(XMFLOAT4 color)
{
	memcpy((char*)constantBufferMap + sizeof(XMFLOAT4X4), &color, sizeof(float) * 4);
}

float euclideanDistance(XMFLOAT2&& l, XMFLOAT2& r)
{
	return  sqrtf( powf(l.x - r.x,2) + powf(l.y - r.y, 2));
}

void Entity::ResolveCollision(CollisionDescriptor& desc)
{

	XMFLOAT2& contact_normal = desc.surfaceNormal;
	float& t_hit_near = desc.t_hit;
	velocity = { velocity.x + contact_normal.x * abs(velocity.x) * (1 - t_hit_near) ,velocity.y + contact_normal.y * abs(velocity.y) * (1 - t_hit_near)};

}
