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
	color.w = opacity;
	scaleMat = DirectX::SimpleMath::Matrix::CreateScale(scale.x, scale.y, 1);
	translationMat = DirectX::SimpleMath::Matrix::CreateTranslation(position);
	Matrix viewRotInv = viewMat;

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
				 viewRotInv *
				translationMat;
		}
		world =  world * parentWorld;
	}
	//view space zorder
	{
		Matrix temp = world * viewMat;
		zorder = temp._43;
	}
	// sprite animation
	{
		if (frameinfo.w > 1)
		{
			_animTimer += deltaTime;
			if (_animTimer >= _animDuration)
			{
				if (_bIsLoop)
				{
					frameinfo.z = static_cast<int>(frameinfo.z + 1) % static_cast<int>(frameinfo.w);
				}
				else
				{
					if (frameinfo.z < frameinfo.w)
						frameinfo.z++;
				}
				_animTimer -= _animDuration;
			}
		}
	}
}
void ParticleEmitter::Initialize(size_t maxParticles, float emissionRate, Vector3 _position)
{
	m_maxParticles = maxParticles;
	m_emissionRate = emissionRate;
	m_emitterPosition = _position;

	m_particlePool.resize(maxParticles);
	worldMat.resize(maxParticles);
	frameinfo.resize(maxParticles);
	color.resize(maxParticles);
	agebuf.resize(maxParticles);
	// 실제 메모리 할당
	//position.resize(maxParticles);
	//velocity.resize(maxParticles);
	//color.resize(maxParticles);
	//scale.resize(maxParticles);
	//opacity.resize(maxParticles);
	//lifetime.resize(maxParticles);
	//age.resize(maxParticles);
	//startColor.resize(maxParticles);
	//endColor.resize(maxParticles);
	//startScale.resize(maxParticles);
	//endScale.resize(maxParticles);
	//startOpacity.resize(maxParticles);
	//endOpacity.resize(maxParticles);
	//animTimer.resize(maxParticles);
	//frameinfo.resize(maxParticles);
	//scaleMat.resize(maxParticles);
	//rotationMat.resize(maxParticles);
	//translationMat.resize(maxParticles);
	//worldMat.resize(maxParticles);
	//zorder.resize(maxParticles);
	//// 인덱스 초기화
	//std::fill(position.begin(), position.end(), Vector3(0, 0, 0));
	//std::fill(velocity.begin(), velocity.end(), Vector3(0, 0, 0));
	//std::fill(color.begin(), color.end(), Vector4(0, 0, 0, 1));
	//std::fill(scale.begin(), scale.end(), Vector2(100, 100));
	//std::fill(opacity.begin(), opacity.end(), 1.f);
	//std::fill(lifetime.begin(), lifetime.end(), 5.f);
	//std::fill(age.begin(), age.end(), 0.f);
	//std::fill(startColor.begin(), startColor.end(), Vector4(1, 1, 1, 1));
	//std::fill(endColor.begin(), endColor.end(), Vector4(0.5f, 0.5f, 0.5f, 1));
	//std::fill(startScale.begin(), startScale.end(), Vector2(3.f, 3.f));
	//std::fill(endScale.begin(), endScale.end(), Vector2(5.f, 5.f));
	//std::fill(startOpacity.begin(), startOpacity.end(), 0.5f);
	//std::fill(endOpacity.begin(), endOpacity.end(), 0);
	//std::fill(animTimer.begin(), animTimer.end(), 0.f);
	//std::fill(frameinfo.begin(), frameinfo.end(), Vector4(1, 1, 0, 1));
	//std::fill(scaleMat.begin(), scaleMat.end(), Matrix::Identity);
	//std::fill(rotationMat.begin(), rotationMat.end(), Matrix::Identity);
	//std::fill(translationMat.begin(), translationMat.end(), Matrix::Identity);
	//std::fill(worldMat.begin(), worldMat.end(), Matrix::Identity);
	//std::fill(zorder.begin(), zorder.end(), 0.f);
	for (size_t i = 0; i < maxParticles; ++i)
	{
		m_particlePool[i].viewMat = viewMat;
		m_particlePool[i].mainCam = mainCam;
		m_inactiveIndices.push(i);
	}

	// 기존 랜덤 초기화 유지
	m_randomGenerator = std::mt19937(m_randomizer());
	m_randomRange0to1 = std::uniform_real_distribution<float>(-1.f, 1.f);
	m_randomVal = std::bind(m_randomRange0to1, m_randomGenerator);
}
void ParticleEmitter::InitializeParticle(int idx)
{
	float randomscale = (m_randomVal() + 2) / 2;
	Vector3 newvel = Vector3(m_randomVal(), m_randomVal(),m_randomVal());

	position[idx] = LocateShape();// +newvel * 50;
	newvel.Normalize();
	velocity[idx] = m_startVelocity *m_speed;
	color[idx] = startColor[idx] = m_startColor;
	scale[idx] = startScale[idx] = m_startScale;
	opacity[idx] = startOpacity[idx] = m_startOpacity;
	lifetime[idx] = m_lifetime;
	age[idx] = 0.0f;
	endColor[idx] = m_endColor;
	endScale[idx] = m_endScale;
	endOpacity[idx] = m_endOpacity;
	frameinfo[idx] = m_startFrame;
}

void ParticleEmitter::InitializeParticle(Particle* particle)
{
	particle->viewMat = viewMat;
	particle->mainCam = mainCam;

	float randomscale = (m_randomVal() + 2) / 2;
	Vector3 newvel = Vector3(m_randomVal(), m_randomVal(), m_randomVal());
	particle->position = LocateShape();// +newvel * 50;
	newvel.Normalize();
	particle->velocity = m_startVelocity *m_speed;
	particle->color = m_startColor;
	particle->m_startColor = m_startColor;
	particle->m_endColor = m_endColor;
	particle->scale = m_startScale;
	particle->m_startScale = m_startScale;
	particle->m_endScale = m_endScale;
	particle->opacity = m_startOpacity;
	particle->m_startOpacity = m_startOpacity;
	particle->m_endOpacity = m_endOpacity;
	particle->lifetime = m_lifetime;
	particle->age = 0.0f;
	particle->frameinfo = m_startFrame;
}

void ParticleEmitter::Update(float deltaTime)
{

	m_translationMat = Matrix::CreateTranslation(m_emitterPosition);
	m_rotationMat = Matrix::CreateFromQuaternion(m_emitterRotation);
	m_worldMat = m_rotationMat * m_translationMat;
	// 활성 파티클 업데이트
#pragma omp parallel for
	for (int i = 0 ; i < m_activeCount;++i)
	{
		//UpdateParticles(deltaTime,i);
		m_particlePool[i].viewMat = viewMat;
		m_particlePool[i].parentWorld = m_worldMat;
		m_particlePool[i].Update(deltaTime);
	}

	// 수명 다한 파티클 비활성화
	for (int i = 0; i < m_activeCount;++i)
	{
		//if (age[i] >= lifetime[i])
		if (m_particlePool[i].age >= m_particlePool[i].lifetime)
		{
			m_particlePool[i].color = Vector4(1, 0, 1, 1);
			m_activeCount--;
			//SwapVectors(i, m_activeCount);
			std::swap(m_particlePool[i], m_particlePool[m_activeCount]);
			m_inactiveIndices.push(m_activeCount);
		}
	}

	ViewSpaceSort();
#pragma omp parallel for
	for (int i = 0;i < m_activeCount;i++)
	{
		worldMat[i] = DirectX::XMMatrixTranspose(m_particlePool[i].world);
		frameinfo[i] = m_particlePool[i].frameinfo;
		color[i] = m_particlePool[i].color;
		agebuf[i] = { m_particlePool[i].age,m_particlePool[i].lifetime,0,0 };
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
		if (index >= m_activeCount) 
		{
			//SwapVectors(index, m_activeCount);
			std::swap(m_particlePool[index], m_particlePool[m_activeCount]);
			index = m_activeCount;
		}
		//InitializeParticle(index);
		m_activeCount++;
		InitializeParticle(&m_particlePool[index]);
		newParticles--;
	}



}

void ParticleEmitter::UpdateParticles(float deltaTime, int idx)
{

	position[idx] += velocity[idx] * deltaTime;
	age[idx] += deltaTime;
	float ratio = age[idx] / lifetime[idx];
	opacity[idx] = ratio * (endOpacity[idx] - startOpacity[idx]) + startOpacity[idx];
	color[idx] = ratio * (endColor[idx] - startColor[idx]) + startColor[idx];
	scale[idx] = ratio * (endScale[idx] - startScale[idx]) + startScale[idx];
	color[idx].w = opacity[idx];
	scaleMat[idx] = DirectX::SimpleMath::Matrix::CreateScale(scale[idx].x, scale[idx].y, 1);
	translationMat[idx] = DirectX::SimpleMath::Matrix::CreateTranslation(position[idx]);
	Matrix viewRotInv = viewMat;

	//billboarding
	{
		if (axis != Vector3::Zero)
		{
			Vector3 toCamera = *mainCam - position[idx];
			toCamera.Normalize();
			axis.Normalize();
			Vector3 right = axis.Cross(toCamera);
			right.Normalize();
			Vector3 forward = right.Cross(axis);
			forward.Normalize();
			worldMat[idx]._11 = right.x;    worldMat[idx]._12 = right.y;    worldMat[idx]._13 = right.z;    // Right 벡터
			worldMat[idx]._21 = axis.x;     worldMat[idx]._22 = axis.y;     worldMat[idx]._23 = axis.z;     // 고정 축 (Up)
			worldMat[idx]._31 = forward.x;  worldMat[idx]._32 = forward.y;  worldMat[idx]._33 = forward.z;  // Forward 벡터
			worldMat[idx]._41 = position[idx].x; worldMat[idx]._42 = position[idx].y; worldMat[idx]._43 = position[idx].z; // 위치
			worldMat[idx] = scaleMat[idx] * worldMat[idx];

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
			worldMat[idx] = scaleMat[idx] *
				rotationMat[idx] * viewRotInv *
				translationMat[idx];
		}
		worldMat[idx] = worldMat[idx] * m_worldMat;
		worldMat[idx] = DirectX::XMMatrixTranspose(worldMat[idx]);
		
	}
	//view space zorder
	{
		Matrix temp = worldMat[idx] * viewMat;
		zorder[idx] = temp._43;
	}
	// sprite animation
	{
		if (frameinfo[idx].w > 1)
		{
			animTimer[idx] += deltaTime;
			if (animTimer[idx] >= _animDuration)
			{
				if (bIsLoop)
				{
					frameinfo[idx].z = static_cast<int>(frameinfo[idx].z + 1) 
						% static_cast<int>(frameinfo[idx].w);
				}
				else
				{
					if (frameinfo[idx].z < frameinfo[idx].w)
						frameinfo[idx].z++;
				}
				animTimer[idx] -= _animDuration;
			}
		}
	}

}

void ParticleEmitter::ViewSpaceSort()
{
//#pragma omp parallel for
//	for (int i = 0; i < m_activeCount;i++)
//	{
//		for (int j = i; j < m_activeCount;j++)
//		{
//			if (zorder[i] > zorder[j])
//			{
//				SwapVectors(i, j);
//			}
//		}
//	}
	std::sort(m_particlePool.begin(), m_particlePool.begin() + m_activeCount,
		[](const Particle& p1, const Particle& p2)->bool
		{
			return p1.zorder > p2.zorder;
		});
}

void ParticleEmitter::SwapVectors(int i, int j)
{
	std::swap(position[i], position[j]);
	std::swap(velocity[i], velocity[j]);
	std::swap(color[i], color[j]);
	std::swap(scale[i], scale[j]);
	std::swap(opacity[i], opacity[j]);
	std::swap(lifetime[i], lifetime[j]);
	std::swap(age[i], age[j]);
	std::swap(startColor[i], startColor[j]);
	std::swap(endColor[i], endColor[j]);
	std::swap(startScale[i], startScale[j]);
	std::swap(endScale[i], endScale[j]);
	std::swap(startOpacity[i], startOpacity[j]);
	std::swap(endOpacity[i], endOpacity[j]);
	std::swap(frameinfo[i], frameinfo[j]);
	std::swap(animTimer[i], animTimer[j]);
	std::swap(scaleMat[i], scaleMat[j]);
	std::swap(rotationMat[i], rotationMat[j]);
	std::swap(translationMat[i], translationMat[j]);
	std::swap(worldMat[i], worldMat[j]);
	std::swap(zorder[i], zorder[j]);
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
	//float idxfloat = m_randomVal() * (vertices.size() - 2);
	//int idx = static_cast<UINT>(idxfloat);
	//return vertices[idx] *(idxfloat-idx)+ vertices[idx+1] * (1+idx-idxfloat);
	Vector3 location = { m_randomVal(),m_randomVal(),m_randomVal()};
	/*while (location.Length() > 1)
	{
		location = { m_randomVal(),m_randomVal(),m_randomVal() };
	}*/
	// + location * randomOffset
	return vertices[m_randomIndex()] + location * randomOffset;
}
void SurfaceEmitter::Update(float deltaTime)
{

	Matrix modelMat = Matrix::CreateScale(modelScale);
	/*m_rotationMat = Matrix::CreateFromQuaternion(m_emitterRotation);*/
	m_worldMat = (*targetModel->objectMatrix) * modelMat;
	// 활성 파티클 업데이트
#pragma omp parallel for
	for (int i = 0; i < m_activeCount;++i)
	{
		//UpdateParticles(deltaTime,i);
		m_particlePool[i].viewMat = viewMat;
		m_particlePool[i].parentWorld = m_worldMat;
		m_particlePool[i].Update(deltaTime);
	}

	// 수명 다한 파티클 비활성화
	for (int i = 0; i < m_activeCount;++i)
	{
		//if (age[i] >= lifetime[i])
		if (m_particlePool[i].age >= m_particlePool[i].lifetime)
		{
			m_particlePool[i].color = Vector4(1, 0, 1, 1);
			m_activeCount--;
			//SwapVectors(i, m_activeCount);
			std::swap(m_particlePool[i], m_particlePool[m_activeCount]);
			m_inactiveIndices.push(m_activeCount);
		}
	}

	ViewSpaceSort();
#pragma omp parallel for
	for (int i = 0;i < m_activeCount;i++)
	{
		worldMat[i] = DirectX::XMMatrixTranspose(m_particlePool[i].world);
		frameinfo[i] = m_particlePool[i].frameinfo;
		color[i] = m_particlePool[i].color;
		agebuf[i] = { m_particlePool[i].age,m_particlePool[i].lifetime,0,0 };
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
		if (index >= m_activeCount)
		{
			//SwapVectors(index, m_activeCount);
			std::swap(m_particlePool[index], m_particlePool[m_activeCount]);
			index = m_activeCount;
		}
		//InitializeParticle(index);
		m_activeCount++;
		InitializeParticle(&m_particlePool[index]);
		newParticles--;
	}


}

