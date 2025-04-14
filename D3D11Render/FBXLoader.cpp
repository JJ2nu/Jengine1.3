#include "pch.h"
#include "FBXLoader.h"

#include <codecvt>
#include <filesystem>
#include <locale>

#include "Model.h"

bool Render::FBXLoader::ImportModel(Render::Model* model, std::wstring modelPath)
{
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);
	unsigned int importFlags = aiProcess_Triangulate |    // vertex »ï°¢Çü À¸·Î Ãâ·Â
		aiProcess_GenNormals |        // Normal Á¤º¸ »ý¼º  
		aiProcess_GenUVCoords |      // ÅØ½ºÃ³ ÁÂÇ¥ »ý¼º
		aiProcess_CalcTangentSpace |  // ÅºÁ¨Æ® º¤ÅÍ »ý¼º
		aiProcess_LimitBoneWeights |
		aiProcess_ConvertToLeftHanded;  // DX¿ë ¿Þ¼ÕÁÂÇ¥°è º¯È¯
	if (model->isStaticMesh)
	{
		importFlags |= aiProcess_PreTransformVertices;
	}


	std::string path(modelPath.begin(), modelPath.end());

	const aiScene* pScene = importer.ReadFile(path, importFlags);

	if (pScene == nullptr)
		return false;

	this->directory = path.substr(0, path.find_last_of("/\\"));

	model->RootNode = new Render::Node();
	if (model->hasBones)
		processBone(pScene);
	processNode(pScene->mRootNode, pScene, model->RootNode, model, nullptr);

	if (!model->hasBones)
	{
		model->m_meshes = std::move(meshes_);
	}
	else
	{
		model->m_skeletalMeshes = std::move(skeletalmeshes_);
		model->m_skeletalInfo = std::move(skeletalinfo_);
	}


	processMaterial(pScene);
	model->m_materials = std::move(textures_);
	if (!model->m_materials.empty() && model->m_materials[0].bUsePBR)
	{
		model->usePBR = true;
	}

	if (pScene->HasAnimations())
	{
		processAnimation(pScene, model);
	}
	model->m_animations = std::move(animations_);

	return true;

}

std::wstring Render::FBXLoader::aiToWstring(const aiString& targetString)
{
	std::string tempstring = targetString.C_Str();
	std::wstring tempwstring;
	tempwstring.assign(tempstring.begin(), tempstring.end());
	return tempwstring;

}

void Render::FBXLoader::Initialize(Render::D3DRenderer* _renderer)
{
	renderer = _renderer;
}

void Render::FBXLoader::processNode(aiNode* node, const aiScene* scene, Render::Node*& dstNode, Model* model, Node* parentNode)
{

	dstNode->m_NodeName = aiToWstring(node->mName);
	dstNode->LocalTransform = Matrix(&node->mTransformation.a1).Transpose();
	dstNode->model = model;
	if (model->hasBones)
	{
		for (UINT i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			Render::SkeletalMesh* tempMesh;
			processSkeletalMesh(mesh, scene, tempMesh, model, parentNode);
			tempMesh->textureIdx = mesh->mMaterialIndex;
			skeletalmeshes_.push_back(tempMesh);
			dstNode->meshes.emplace_back(skeletalmeshes_.size() - 1);

			for (auto& boneref : tempMesh->m_BoneReferences)
			{
				boneref.nodeWorldMatrixPtr = &dstNode->LocalTransform;
			}
		}
	}
	else
	{
		for (UINT i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			Render::Mesh* tempMesh;
			processMesh(mesh, scene, tempMesh, model);
			tempMesh->textureIdx = mesh->mMaterialIndex;
			meshes_.push_back(tempMesh);
			dstNode->meshes.emplace_back(meshes_.size() - 1);
		}
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		Render::Node* childNode = new Render::Node();
		childNode->parentNode = dstNode;
		processNode(node->mChildren[i], scene, childNode, model, dstNode);
		dstNode->childNodes.push_back(childNode);
	}
}

void Render::FBXLoader::processMesh(aiMesh* mesh, const aiScene* scene, Render::Mesh*& dstMesh, Model*& model)
{
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;
	std::vector<Texture> textures;

	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex(Vector4(0, 0, 0, 0), Vector3(0, 0, 0),
			Vector3(0, 0, 0), Vector3(0, 0, 0), Vector2(0, 0));

		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;
		vertex.position.w = 1.f;

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		if (mesh->HasTangentsAndBitangents())
		{
			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;

			vertex.bitangent.x = mesh->mBitangents[i].x;
			vertex.bitangent.y = mesh->mBitangents[i].y;
			vertex.bitangent.z = mesh->mBitangents[i].z;
		}

		if (mesh->mTextureCoords[0]) {
			vertex.tex.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.tex.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	dstMesh = new Mesh(vertices, indices);
}

void Render::FBXLoader::processSkeletalMesh(aiMesh* mesh, const aiScene* scene, Render::SkeletalMesh*& dstMesh, Model*& model, Node* parentNode)
{
	std::vector<BoneWeightVertex> vertices;
	std::vector<UINT> indices;
	std::vector<Texture> textures;
	vertices.resize(mesh->mNumVertices);
	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		vertices[i].position = Vector4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.f);
		vertices[i].normal = Vector3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		if (mesh->HasTangentsAndBitangents())
		{

			vertices[i].tangent = Vector3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			vertices[i].bitangent = Vector3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}
		if (mesh->mTextureCoords[0])
		{
			vertices[i].tex.x = (float)mesh->mTextureCoords[0][i].x;
			vertices[i].tex.y = (float)mesh->mTextureCoords[0][i].y;
		}
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	SkeletalMesh* tempSkeletalMesh = new SkeletalMesh(vertices, indices);
	UINT meshBoneCount = mesh->mNumBones;
	std::vector<BoneRef> BoneRefs;
	BoneRefs.resize(meshBoneCount);

	for (UINT i = 0; i < meshBoneCount; ++i)
	{
		aiBone* pAiBone = mesh->mBones[i];
		UINT boneIndex = skeletalinfo_->GetBoneIndexByName(aiToWstring(pAiBone->mName));
		assert(boneIndex != -1);
		BoneInfo* pBone = skeletalinfo_->GetBoneInfoByIndex(boneIndex);
		assert(pBone != nullptr);
		pBone->OffsetMatrix = Matrix(&pAiBone->mOffsetMatrix.a1).Transpose();
		BoneRefs[i].nodeName = aiToWstring(pAiBone->mName);
		BoneRefs[i].boneIdx = boneIndex;
		for (UINT j = 0; j < pAiBone->mNumWeights; ++j)
		{
			UINT vertexID = pAiBone->mWeights[j].mVertexId;
			float weight = pAiBone->mWeights[j].mWeight;
			tempSkeletalMesh->m_BoneVertices[vertexID].AddBoneData(boneIndex, weight);
		}

	}

	dstMesh = std::move(tempSkeletalMesh);
	dstMesh->m_pMatrixPalletePtr = renderer->m_pMatrixPallete;
	dstMesh->m_BoneReferences = std::move(BoneRefs);
	dstMesh->m_pSkeletalInfo = skeletalinfo_;
}

void Render::FBXLoader::processBone(const aiScene* scene)
{
	skeletalinfo_ = new SkeletalInfo();
	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		UINT meshBoneCount = mesh->mNumBones;
		if (skeletalinfo_->bones.size() < meshBoneCount)
			skeletalinfo_->bones.resize(meshBoneCount);

		//create skeletoninfo
		for (UINT i = 0; i < meshBoneCount; ++i)
		{
			aiBone* pAiBone = mesh->mBones[i];
			std::wstring boneName = aiToWstring(pAiBone->mName);
			Matrix offsetMatrix = Matrix(&pAiBone->mOffsetMatrix.a1).Transpose();
			BoneInfo* boneInfo = new BoneInfo();
			boneInfo->BoneName = boneName;
			boneInfo->OffsetMatrix = offsetMatrix;
			boneInfo->m_pSkeletalInfo = skeletalinfo_;
			skeletalinfo_->bones[i] = std::move(boneInfo);
			skeletalinfo_->BoneMappingTable[boneName] = i;
		}
	}
}

void Render::FBXLoader::processMaterial(const aiScene* pScene)
{
	for (UINT i = 0; i < pScene->mNumMaterials; i++)
	{
		Render::Texture tempTex;
		aiString texturePath;
		std::string tempString;
		aiMaterial* pMaterial = pScene->mMaterials[i];
		if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath))
		{
			std::string temp(texturePath.C_Str());
			std::filesystem::path fsPath(temp);
			std::string fileName = fsPath.filename().string(); // "eyes_diff.png"

			std::string result = directory + "/" + fileName;
			tempTex.m_diffusePath.assign(result.begin(), result.end());
			renderer->CreateMaterial(tempTex.m_diffusePath, tempTex.m_diffuseRV);
		}


		else if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath))
		{
			std::string temp(texturePath.C_Str());
			std::filesystem::path fsPath(temp);
			std::string fileName = fsPath.filename().string(); // "eyes_diff.png"

			std::string result = directory + "/" + fileName;
			tempTex.m_diffusePath.assign(result.begin(), result.end());
			renderer->CreateMaterial(tempTex.m_diffusePath, tempTex.m_diffuseRV);
		}

		else
			renderer->CreateMaterial(L"", tempTex.m_diffuseRV);

		if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_NORMALS, 0, &texturePath))
		{
			std::string temp(texturePath.C_Str());
			std::filesystem::path fsPath(temp);
			std::string fileName = fsPath.filename().string(); // "eyes_diff.png"

			std::string result = directory + "/" + fileName;
			tempTex.m_normalPath.assign(result.begin(), result.end());
			renderer->CreateMaterial(tempTex.m_normalPath, tempTex.m_normalRV);
		}
		else
			renderer->CreateMaterial(L"", tempTex.m_normalRV);

		if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &texturePath))
		{
			std::string temp(texturePath.C_Str());
			std::filesystem::path fsPath(temp);
			std::string fileName = fsPath.filename().string(); // "eyes_diff.png"

			std::string result = directory + "/" + fileName;
			tempTex.m_specularPath.assign(result.begin(), result.end());
			renderer->CreateMaterial(tempTex.m_specularPath, tempTex.m_specularRV);
		}
		else
			renderer->CreateMaterial(L"", tempTex.m_specularRV);
		if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath))
		{
			std::string temp(texturePath.C_Str());
			std::filesystem::path fsPath(temp);
			std::string fileName = fsPath.filename().string(); // "eyes_diff.png"

			std::string result = directory + "/" + fileName;
			tempTex.m_emissivePath.assign(result.begin(), result.end());
			renderer->CreateMaterial(tempTex.m_emissivePath, tempTex.m_emissiveRV);
		}
		else
			renderer->CreateMaterial(L"", tempTex.m_emissiveRV);
		if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_OPACITY, 0, &texturePath))
		{
			std::string temp(texturePath.C_Str());
			std::filesystem::path fsPath(temp);
			std::string fileName = fsPath.filename().string(); // "eyes_diff.png"

			std::string result = directory + "/" + fileName;
			tempTex.m_opacityPath.assign(result.begin(), result.end());
			renderer->CreateMaterial(tempTex.m_opacityPath, tempTex.m_opacityRV);
		}
		else
			renderer->CreateMaterial(L"", tempTex.m_opacityRV);

		if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_METALNESS, 0, &texturePath))
		{
			std::string temp(texturePath.C_Str());
			std::filesystem::path fsPath(temp);
			std::string fileName = fsPath.filename().string(); // "eyes_diff.png"

			std::string result = directory + "/" + fileName;
			tempTex.m_metalnessPath.assign(result.begin(), result.end());
			renderer->CreateMaterial(tempTex.m_metalnessPath, tempTex.m_metalnessRV);
			tempTex.bUsePBR = true;
		}
		else
			renderer->CreateMaterial(L"", tempTex.m_metalnessRV);

		if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_SHININESS, 0, &texturePath))
		{
			std::string temp(texturePath.C_Str());
			std::filesystem::path fsPath(temp);
			std::string fileName = fsPath.filename().string(); // "eyes_diff.png"

			std::string result = directory + "/" + fileName;
			tempTex.m_roughnessPath.assign(result.begin(), result.end());
			renderer->CreateMaterial(tempTex.m_roughnessPath, tempTex.m_roughnessRV);
			tempTex.bUsePBR = true;
		}
		else
			renderer->CreateMaterial(L"", tempTex.m_roughnessRV);



		textures_.push_back(tempTex);
	}
}

void Render::FBXLoader::processAnimation(const aiScene* pScene, Model*& model)
{
	Render::Animation tempAnimation;

	for (size_t i = 0; i < pScene->mNumAnimations; i++)
	{
		aiAnimation* tempAiAnim = pScene->mAnimations[i];
		tempAnimation.name = aiToWstring(tempAiAnim->mName);

		tempAnimation.m_Duration = tempAiAnim->mDuration;
		tempAnimation.m_TPS = tempAiAnim->mTicksPerSecond;
		tempAnimation.m_totalTime = tempAnimation.m_Duration / tempAnimation.m_TPS;

		for (size_t j = 0; j < tempAiAnim->mNumChannels; j++)
		{
			Render::NodeAnim* tempNodeAnim = new Render::NodeAnim();
			aiNodeAnim* tempAiNodeAnim = tempAiAnim->mChannels[j];
			tempNodeAnim->nodeName = aiToWstring(tempAiNodeAnim->mNodeName);
			model->RootNode->FindNode(tempNodeAnim->nodeName)->nodeanim = tempNodeAnim;
			for (size_t nodeAnimIdx = 0; nodeAnimIdx < tempAiNodeAnim->mNumPositionKeys; nodeAnimIdx++)
			{
				tempNodeAnim->time.push_back(tempAiNodeAnim->mPositionKeys[nodeAnimIdx].mTime);
				tempNodeAnim->translationKey.push_back(Vector3(tempAiNodeAnim->mPositionKeys[nodeAnimIdx].mValue.x, tempAiNodeAnim->mPositionKeys[nodeAnimIdx].mValue.y, tempAiNodeAnim->mPositionKeys[nodeAnimIdx].mValue.z));
				tempNodeAnim->rotationKey.push_back(Quaternion(tempAiNodeAnim->mRotationKeys[nodeAnimIdx].mValue.x, tempAiNodeAnim->mRotationKeys[nodeAnimIdx].mValue.y, tempAiNodeAnim->mRotationKeys[nodeAnimIdx].mValue.z, tempAiNodeAnim->mRotationKeys[nodeAnimIdx].mValue.w));
				tempNodeAnim->scaleKey.push_back(Vector3(tempAiNodeAnim->mScalingKeys[nodeAnimIdx].mValue.x, tempAiNodeAnim->mScalingKeys[nodeAnimIdx].mValue.y, tempAiNodeAnim->mScalingKeys[nodeAnimIdx].mValue.z));
			}
			tempAnimation.nodeChannels.push_back(*tempNodeAnim);
		}
		animations_.push_back(tempAnimation);
	}




}
