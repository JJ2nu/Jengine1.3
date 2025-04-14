#include "pch.h"
#include "Model.h"
#include "Particles.h"

void Particle::Update(float deltaTime)
{


	scaleMat = DirectX::SimpleMath::Matrix::CreateScale(scale.x, scale.y, 1);
	translationMat = DirectX::SimpleMath::Matrix::CreateTranslation(position);
	Matrix viewRotInv = *viewMat;
	_frameinfo.blendColor = color;
	_frameinfo.blendColor.w = opacity;

	//billboarding
	{
		if (axis != Vector3::Zero)
		{
			Vector3 toCamera = mainCam - position;
			toCamera.Normalize();
			axis.Normalize();
			Vector3 right = axis.Cross(toCamera);
			right.Normalize();
			Vector3 forward = right.Cross(axis);
			forward.Normalize();

			world._11 = right.x;    world._12 = right.y;    world._13 = right.z;    // Right 벡터
			world._21 = axis.x;     world._22 = axis.y;     world._23 = axis.z;     // 고정 축 (Up)
			world._31 = forward.x;  world._32 = forward.y;  world._33 = forward.z;  // Forward 벡터
			world._41 = position.x; world._42 = position.y; world._43 = position.z; // 위치
			world = scaleMat * world;

		}
		else
		{
			viewRotInv._14 = 0.0f;
			viewRotInv._24 = 0.0f;
			viewRotInv._34 = 0.0f;
			viewRotInv._41 = 0.0f;
			viewRotInv._42 = 0.0f;
			viewRotInv._43 = 0.0f;
			viewRotInv._44 = 1.0f;
			viewRotInv = viewRotInv.Invert();
			world = scaleMat * 
				rotationMat * viewRotInv *
				translationMat;
		}
		world =  world * parentWorld;
	}
	//view space zorder
	{
		Vector3 s, t;
		Quaternion q;
		Matrix temp = world * (*viewMat);
		temp.Decompose(s, q, t);
		zorder = t.z;
	}
	// sprite animation
	{
		if (_frameinfo.frameCnt.w > 1)
		{
			_animTimer += deltaTime;
			if (_animTimer >= _animDuration)
			{
				if (_bIsLoop)
				{
					_frameinfo.frameCnt.z = static_cast<int>(_frameinfo.frameCnt.z + 1) % static_cast<int>(_frameinfo.frameCnt.w);
				}
				else
				{
					if (_frameinfo.frameCnt.z < _frameinfo.frameCnt.w)
						_frameinfo.frameCnt.z++;
				}
				_animTimer -= _animDuration;
			}
		}
	}

}



void ParticleEmitter::Initialize(size_t maxParticles, float emissionRate, Vector3 position)
{
    m_maxParticles = maxParticles;
    m_emissionRate = emissionRate;
    m_emitterPosition = position;

	for (size_t i = 0; i < maxParticles; ++i) {
		Particle* p = new Particle();
		p->active = false;
		p->parentEmitter = this;
		m_particlePool.push_back(p);
	}
	//for (auto& p : m_particlePool) {
	//	m_inactiveParticles.push(p);
	//}

    m_randomGenerator = std::mt19937(m_randomizer());
    m_randomRange0to1 = std::uniform_real_distribution<float>(-1.f,1.f);
	m_randomVal = std::bind(m_randomRange0to1, m_randomGenerator);
}

void ParticleEmitter::InitializeParticle(Particle* particle)
{
	particle->parentWorld = particle->parentEmitter->m_worldMat;
	float randomscale = (m_randomVal() + 2) / 2;
	Vector3 newvel = Vector3(m_randomVal(), m_randomVal(),m_randomVal());
	particle->position = LocateShape() + newvel*10 ;
	newvel.Normalize();

	//particle->velocity = (newvel*0.33f+m_startVelocity * 0.66f) *150.f * randomscale;
	particle->velocity = m_startVelocity*100.f;
	particle->color = m_startColor;
	particle->scale = m_startScale;
	particle->opacity = m_startOpacity;
	particle->lifetime = m_lifetime;
	particle->age = 0.0f;
	particle->mainCam = mainCam;
	particle->viewMat = viewMat;
}

void ParticleEmitter::Update(float deltaTime)
{


	// 활성 파티클 업데이트
	for (auto it = m_activeParticles.begin(); it != m_activeParticles.end();) {
		Particle* particle = *it;


		particle->position += particle->velocity * deltaTime;
		particle->age += deltaTime;
		particle->opacity = particle->age / m_lifetime * (m_endOpacity - m_startOpacity) + m_startOpacity;
		particle->color = particle->age / m_lifetime * (m_endColor - m_startColor) +m_startColor;
		particle->scale = particle->age / m_lifetime * (m_endScale - m_startScale) + m_startScale;
		particle->Update(deltaTime);

		if (particle->age >= particle->lifetime) {
			// 비활성 처리 및 풀 반환
			particle->active = false;
			it = m_activeParticles.erase(it);
		}
		else {
			++it;
		}
	}

	// 새 파티클 생성
	size_t newParticles = 0;
	m_emissionThreshold += deltaTime * m_emissionRate;
	if (m_emissionThreshold >= 1)
	{
		newParticles = static_cast<size_t>(m_emissionThreshold);
		m_emissionThreshold -= newParticles;
	}

	// 풀에서 비활성 파티클 재사용
	for (auto& particle : m_particlePool) {
		if (newParticles == 0) break;

		if (!particle->active) {
			InitializeParticle(particle);
			particle->active = true;
			m_activeParticles.push_back(particle);
			newParticles--;
		}
	}

	m_translationMat = Matrix::CreateTranslation(m_emitterPosition);
	m_rotationMat = Matrix::CreateFromQuaternion(m_emitterRotation);
	m_worldMat = m_rotationMat * m_translationMat;
}



//////////////////////////////////////////////////////////////////////////////////////////////////
bool ParticleSystem::Initialize()
{
	vertices.resize(4);
	indices.resize(6);
	vertices[0].position = Vector4(-1.f, 1.f, 0.0f, 1.f); // Top left
	vertices[0].normal = Vector3(0, 0, -1);
	vertices[0].tangent = Vector3(0, 1, 0);
	vertices[0].bitangent = Vector3(1, 0, 0);
	vertices[0].tex = Vector2(0, 0); // Top left

	vertices[1].position = Vector4(1.f, 1.f, 0.0f, 1.f); // Top right
	vertices[1].normal = Vector3(0, 0, -1);
	vertices[1].tangent = Vector3(0, 1, 0);
	vertices[1].bitangent = Vector3(1, 0, 0);
	vertices[1].tex = Vector2(1, 0); // Top right

	vertices[2].position = Vector4(-1.f, -1.f, 0.0f, 1.f); // Bottom left
	vertices[2].normal = Vector3(0, 0, -1);
	vertices[2].tangent = Vector3(0, 1, 0);
	vertices[2].bitangent = Vector3(1, 0, 0);
	vertices[2].tex = Vector2(0, 1); // Bottom left

	vertices[3].position = Vector4(1.f, -1.f, 0.0f, 1.f); // Bottom right
	vertices[3].normal = Vector3(0, 0, -1);
	vertices[3].tangent = Vector3(0, 1, 0);
	vertices[3].bitangent = Vector3(1, 0, 0);
	vertices[3].tex = Vector2(1, 1); // Bottom right
	indices = { 3,1,2,2,1,0 };
	return true;
}

void ParticleSystem::Update(float deltaTime)
{
    for (auto emitter : m_emitters)
    {
		if (false == emitter->m_Active) continue;
		emitter->mainCam = mainCam;
		emitter->viewMat = viewMat;
        emitter->Update(deltaTime);
    }
}

ParticleEmitter* ParticleSystem::CreateEmitter(LocationShape locationshape, const std::vector<Vector3>& vertices)
{
	ParticleEmitter* newPE;
	switch (locationshape)
	{
	case LocationShape::SPHERE:
		newPE = new SphereEmitter();
		break;
	case LocationShape::CUBE:
		newPE = new CubeEmitter();
		break;
	case LocationShape::CYLINDER:
		newPE = new CylinderEmitter();
		break;
	case LocationShape::CONE:
		newPE = new ConeEmitter();
		break;
	case LocationShape::TORUS:
		newPE = new TorusEmitter();
		break;
	case LocationShape::SURFACE:
		newPE = new SurfaceEmitter(vertices);
		break;
	default:
		newPE = new SphereEmitter();

		break;
	}
	newPE->parentSys = this;
	newPE->Initialize();
	m_emitters.push_back(newPE);
	return newPE;
}

void ParticleSystem::EnableEmitter(size_t index)
{
	if (nullptr != m_emitters[index])
		m_emitters[index]->m_Active = true;
}

void ParticleSystem::RemoveEmitter(size_t index)
{
    ///TODO: remove specific emitter in m_emitters via index
}
//////////////////////////////////////////////////////////////////////////////////////////////////

DirectX::SimpleMath::Vector3 SphereEmitter::LocateShape()
{
    Vector3 location = { m_randomVal(),m_randomVal(),m_randomVal() };
    while (location.Length() > 1)
    {
        location = { m_randomVal(),m_randomVal(),m_randomVal() };
    }
    return location*radius;
}

DirectX::SimpleMath::Vector3 CubeEmitter::LocateShape()
{
	return Vector3(m_randomVal() * scale.x, m_randomVal() * scale.y, m_randomVal() * scale.z);
}

DirectX::SimpleMath::Vector3 CylinderEmitter::LocateShape()
{

	Vector2 location = { m_randomVal(),m_randomVal() };
	while (location.Length() > 1)
	{
		location = { m_randomVal(),m_randomVal() };
	}
	return Vector3(location.x * radius, m_randomVal() * height / 2, location.y * radius);

}
DirectX::SimpleMath::Vector3 ConeEmitter::LocateShape()
{
	float locationY = (m_randomVal() +2/2)* height;
	float sectionRadius = locationY * std::tan(angle);
	Vector3 location = Vector3(m_randomVal(),0, m_randomVal());
	while (location.Length() > 1)
	{
		location = { m_randomVal(),0,m_randomVal() };
	}
	location *= sectionRadius;
	location.y = locationY;
	return location;
}

DirectX::SimpleMath::Vector3 TorusEmitter::LocateShape()
{
	Vector3 location = Vector3(m_randomVal(), 0, m_randomVal()) * outerRadius;
	while (location.Length() > outerRadius || location.Length() < innerRadius)
	{
		location = Vector3(m_randomVal(), 0, m_randomVal()) * outerRadius;
	}
	if (false == isDisc)
	{
		float length = location.Length();
		float range = std::sqrtf(outerRadius * (outerRadius - 2 * innerRadius) - length * (length - 2 * innerRadius));
		location.y = m_randomVal() * range;
	}
	return location;
}


void SurfaceEmitter::Initialize(size_t maxParticles, float emissionRate, Vector3 position)
{
	ParticleEmitter::Initialize(maxParticles, emissionRate, position);
	m_randomRangeVertexIndex = std::uniform_int_distribution<int>(0, vertices.size()-1);
	m_randomIndex = std::bind(m_randomRangeVertexIndex, m_randomGenerator);
}
Vector3 SurfaceEmitter::LocateShape()
{
	int idx = m_randomIndex();
	return vertices[idx];
}
void SurfaceEmitter::Update(float deltaTime)
{

	m_worldMat = *targetModel->objectMatrix;

	// 활성 파티클 업데이트
	for (auto it = m_activeParticles.begin(); it != m_activeParticles.end();) {
		Particle* particle = *it;


		particle->position += particle->velocity * deltaTime;
		particle->age += deltaTime;
		particle->opacity = particle->age / m_lifetime * (m_endOpacity - m_startOpacity) + m_startOpacity;
		particle->color = particle->age / m_lifetime * (m_endColor - m_startColor) + m_startColor;
		particle->scale = particle->age / m_lifetime * (m_endScale - m_startScale) + m_startScale;
		particle->Update(deltaTime);

		if (particle->age >= particle->lifetime) {
			// 비활성 처리 및 풀 반환
			particle->active = false;
			it = m_activeParticles.erase(it);
		}
		else {
			++it;
		}
	}

	// 새 파티클 생성
	size_t newParticles = 0;
	m_emissionThreshold += deltaTime * m_emissionRate;
	if (m_emissionThreshold >= 1)
	{
		newParticles = static_cast<size_t>(m_emissionThreshold);
		m_emissionThreshold -= newParticles;
	}

	// 풀에서 비활성 파티클 재사용
	for (auto& particle : m_particlePool) {
		if (newParticles == 0) break;

		if (!particle->active) {
			InitializeParticle(particle);
			particle->active = true;
			m_activeParticles.push_back(particle);
			newParticles--;
		}
	}


}

