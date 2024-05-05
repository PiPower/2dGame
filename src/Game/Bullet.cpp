#include "Bullet.h"

Bullet::Bullet(DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale)
	:
	Entity(device, origin, scale), velocityBuffer(0,0)
{
}

void Bullet::ResolveCollision(CollisionDescriptor& desc)
{
	if (desc.surfaceNormal.x == 1.0f && desc.surfaceNormal.y == 0.0f)
	{
		velocityBuffer = { -velocity.x, velocity.y };
	}
	else if (desc.surfaceNormal.x == -1.0f && desc.surfaceNormal.y == 0.0f)
	{
		velocityBuffer = { -velocity.x, velocity.y };
	}
	else if (desc.surfaceNormal.x == 0.0f && desc.surfaceNormal.y == 1.0f)
	{
		velocityBuffer = { velocity.x, -velocity.y };
	}
	else if (desc.surfaceNormal.x == 0.0f && desc.surfaceNormal.y == -1.0f)
	{
		velocityBuffer = { velocity.x, -velocity.y };
	}
	else
	{
		MessageBox(NULL, L"bullet collision resolution expects axis aligned normal which is not given!! \n", NULL, MB_OK);
		exit(-1);
	}

	float& t_hit_near = desc.t_hit;
	velocity = { velocity.x * t_hit_near ,velocity.y * t_hit_near };
}

void Bullet::UpdatePosition(bool keepVelocity)
{
	if (velocityBuffer.x != 0 || velocityBuffer.y != 0)
	{
		velocity = velocityBuffer;
		velocityBuffer = { 0,0 };
	}

	this->Entity::UpdatePosition(keepVelocity);
}