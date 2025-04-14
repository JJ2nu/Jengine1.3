#include "pch.h"
#include "D3D11Render.h"
#include "Model.h"


void Render::SkeletalMesh::UpdateMatrixPalette()
{
	assert(m_BoneReferences.size() < 128);
	for (UINT i = 0; i < m_BoneReferences.size(); ++i)
	{
		Matrix& BoneNodeWorldMatrix =*m_BoneReferences[i].nodeWorldMatrixPtr;
		int index = m_BoneReferences[i].boneIdx;
		BoneInfo* pBoneInfo = m_pSkeletalInfo->GetBoneInfoByIndex(index);
		assert(pBoneInfo != nullptr);
		m_pMatrixPalletePtr->Array[index] = (pBoneInfo->OffsetMatrix * BoneNodeWorldMatrix).Transpose();
	}
}

void Render::NodeAnim::Evaluate(double _time, Vector3& s, Quaternion& r, Vector3& t)
{
	UINT timeIdx = 0;
	for(timeIdx = 0 ; timeIdx < this->time.size();timeIdx++)
	{
		if(time[timeIdx]<_time && timeIdx<time.size()-1&& time[timeIdx+1] > _time)
		{
			s = DirectX::SimpleMath::Vector3::Lerp(scaleKey[timeIdx], scaleKey[timeIdx + 1], _time - time[timeIdx]);
			r = DirectX::SimpleMath::Quaternion::Slerp(rotationKey[timeIdx], rotationKey[timeIdx + 1], _time - time[timeIdx]);
			t = DirectX::SimpleMath::Vector3::Lerp(translationKey[timeIdx], translationKey[timeIdx + 1], _time - time[timeIdx]);
			break;
		}
		else if (time[timeIdx] == _time|| timeIdx >= time.size() - 1)
		{
			s = Vector3(&scaleKey[timeIdx].x);
			r = Quaternion(&rotationKey[timeIdx].x);
			t = Vector3(&translationKey[timeIdx].x);
			break;
		}
	}
}

void Render::Node::Update(float deltatime)
{
	if(nodeanim)
	{
		Vector3 position, scale;
		Quaternion rotation;
		float time = progressAnimTime * this->model->m_animations[this->model->curAnimationIdx].m_Duration;
		nodeanim->Evaluate(time, scale, rotation, position);
		 
		LocalTransform = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
	}



	if (parentNode)
		ParentWorldTransform = parentNode->WorldTransform;

	WorldTransform = LocalTransform * ParentWorldTransform;

	for(auto& childNode : childNodes)
	{
		childNode->progressAnimTime = progressAnimTime;
		childNode->Update(deltatime);
	}
	
}

Render::Node* Render::Node::FindNode(std::wstring name)
{
	if (this->m_NodeName == name) return this;
	else
	{
		for(auto node : childNodes)
		{
			Node* temp = node->FindNode(name);
			if (temp&&temp->m_NodeName == name) 
				return temp;
		}
		return nullptr;
	}
}



void Render::Model::Update(float deltatime)
{
	RootNode->ParentWorldTransform = *objectMatrix;
	if (!m_animations.empty())
	{
		RootNode->progressAnimTime = progressAnimTime;
		curTPS = m_animations[curAnimationIdx].m_TPS;

		currentTime += deltatime;
		progressAnimTime = currentTime / static_cast<float>(m_animations[curAnimationIdx].m_totalTime);
		if (progressAnimTime >= 1.f)
		{
			progressAnimTime = 1.f;
			currentTime = 0.f;
		}
	}

	RootNode->Update(deltatime);

	if(hasBones)
	{
		
 		for(auto& skelMesh : m_skeletalMeshes)
		{
			size_t size = skelMesh->m_BoneReferences.size();
			for (size_t i=0;i<size;++i)
			{
				Node* pNode = FindNodeByName(skelMesh->m_BoneReferences[i].nodeName);
				assert(pNode != nullptr);
				skelMesh->m_BoneReferences[i].nodeWorldMatrixPtr = &pNode->WorldTransform;
			}
			skelMesh->UpdateMatrixPalette();
		}

	}


}

Render::Node* Render::Model::FindNodeByName(std::wstring name)
{
	return RootNode->FindNode(name);
}

