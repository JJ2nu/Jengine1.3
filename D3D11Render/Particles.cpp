#include "pch.h"
#include "Model.h"
#include "Particles.h"

void Particle::Update(float deltaTime)
{
	position += velocity * deltaTime;
	age += deltaTime;
	opacity = age / m_lifetime * (m_endOpacity - m_startOpacity) + m_startOpacity;
	color = age / m_lifetime * (m_endColor - m_startColor) + m_startColor;
	scale = age / m_lifetime * (m_endScale - m_startScale) + m_startScale;

	scaleMat = DirectX::SimpleMath::Matrix::CreateScale(scale.x, scale.y, 1);
	translationMat = DirectX::SimpleMath::Matrix::CreateTranslation(position);
	Matrix viewRotInv = *viewMat;
	_frameinfo.blendColor = color;
	_frameinfo.blendColor.w = opacity;

	//billboarding
	{
		if (axis != Vector3::Zero)
		{
			Vector3 toCamera = *mainCam - position;
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
		Matrix temp = world * (*viewMat);
		zorder = temp._43;
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

	m_particlePool.resize(maxParticles);  // 실제 메모리 할당

	// 인덱스 초기화
	for (size_t i = 0; i < maxParticles; ++i) {
		m_particlePool[i].active = false;
		m_particlePool[i].parentEmitter = this;
		m_particlePool[i].index = i;
		m_particlePool[i].viewMat = viewMat;
		m_inactiveIndices.push(i);
	}

	// 기존 랜덤 초기화 유지
	m_randomGenerator = std::mt19937(m_randomizer());
	m_randomRange0to1 = std::uniform_real_distribution<float>(-1.f, 1.f);
	m_randomVal = std::bind(m_randomRange0to1, m_randomGenerator);
}
void ParticleEmitter::InitializeParticle(Particle* particle)
{
	particle->parentWorld = particle->parentEmitter->m_worldMat;
	float randomscale = (m_randomVal() + 2) / 2;
	Vector3 newvel = Vector3(m_randomVal(), m_randomVal(),m_randomVal());
	particle->position = LocateShape() /*+ newvel*10*/ ;
	newvel.Normalize();
	particle->velocity = m_startVelocity*m_speed;
	particle->color = m_startColor;
	particle->scale = m_startScale;
	particle->opacity = m_startOpacity;
	particle->lifetime = m_lifetime;
	particle->age = 0.0f;
	particle->mainCam = mainCam;
	particle->viewMat = viewMat;
	particle->m_startVelocity = m_startVelocity;
	particle->m_startColor = m_startColor;
	particle->m_endColor = m_endColor;
	particle->m_startScale = m_startScale;
	particle->m_endScale = m_endScale;
	particle->m_startOpacity = m_startOpacity;
	particle->m_endOpacity = m_endOpacity;

	particle->_frameinfo = m_startFrame;
	particle->_frameinfo.frameCnt.z = static_cast<size_t>((m_randomVal() + 2) / 2 * particle->_frameinfo.frameCnt.w);
	if (particle->_frameinfo.frameCnt.z == m_startFrame.frameCnt.w)
		particle->_frameinfo.frameCnt.z = 0;
	particle->axis = axis;
}
void ParticleEmitter::Update(float deltaTime)
{
	// 활성 파티클 업데이트
#pragma omp parallel for
	for (int i = 0 ; i < m_activeCount;++i)
	{
		m_particlePool[i].viewMat = viewMat;
		m_particlePool[i].mainCam = mainCam;
		m_particlePool[i].cameraUp = cameraUp;
		m_particlePool[i].Update(deltaTime);

	}

	// 수명 다한 파티클 비활성화
	for (int i = 0; i < m_activeCount;++i)
	{
		if (m_particlePool[i].age >= m_particlePool[i].lifetime)
		{
			m_particlePool[i].active = false;
			m_activeCount--;
			std::swap(m_particlePool[i], m_particlePool[m_activeCount]);
			m_inactiveIndices.push(m_activeCount);
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
	while (0 < newParticles && !m_inactiveIndices.empty())
	{
		size_t index = m_inactiveIndices.top();
		m_inactiveIndices.pop();
		if (index >= m_activeCount) {
			std::swap(m_particlePool[index], m_particlePool[m_activeCount]);
			index = m_activeCount;
		}
		m_activeCount++;
		m_particlePool[index].active = true;
		InitializeParticle(&m_particlePool[index]);
		newParticles--;

	}


	m_translationMat = Matrix::CreateTranslation(m_emitterPosition);
	m_rotationMat = Matrix::CreateFromQuaternion(m_emitterRotation);
	m_worldMat = m_rotationMat * m_translationMat;
	std::sort(m_particlePool.begin(),
		m_particlePool.begin() + m_activeCount,
		[](const Particle& a, const Particle& b) {
			return a.zorder > b.zorder; // 높은 zorder가 먼저 오도록
		});

	for (int i = 0;i < m_activeCount;i++)
	{
		m_instances[i].World = DirectX::XMMatrixTranspose(m_particlePool[i].world);
		m_instances[i].AnimParams = m_particlePool[i]._frameinfo.frameCnt;
		m_instances[i].Color = m_particlePool[i]._frameinfo.blendColor;
	}


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
		emitter->cameraUp = cameraUp;
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
	ParticleEmitter::Update(deltaTime);
	m_worldMat = *targetModel->objectMatrix;
}

