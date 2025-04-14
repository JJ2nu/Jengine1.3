#pragma once

#include "../Utility/SingletonBase.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "D3D11Render.h"
#include "Model.h"

namespace Render
{
	class FBXLoader 
	{
		/*std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_TextureMap;
		std::unordered_map<std::wstring, std::pair< Microsoft::WRL::ComPtr<ID3D11VertexShader>, Microsoft::WRL::ComPtr<ID3D11InputLayout>>> m_VertexShaderMap;
		std::unordered_map<std::wstring,  Microsoft::WRL::ComPtr<ID3D11PixelShader>> m_PixelShaderMap;
		std::unordered_map<std::wstring, std::shared_ptr<Render::Mesh>> m_MeshMap;*/

		Assimp::Importer importer; // 기본 임포트 옵션

		std::string directory;
		std::vector<Mesh*> meshes_;
		std::vector<SkeletalMesh*> skeletalmeshes_;
		std::vector<Texture> textures_;
		std::vector<Animation> animations_;
		SkeletalInfo* skeletalinfo_;

		void processNode(aiNode* node, const aiScene* scene, Render::Node*& dstNode, Model* model, Node* parentNode);
		void processMesh(aiMesh* mesh, const aiScene* scene , Render::Mesh*& dstMesh, Model*& model);
		void processSkeletalMesh(aiMesh* mesh, const aiScene* scene, Render::SkeletalMesh*& dstMesh, Model*& model, Node* parentNode);
		void processBone( const aiScene* scene);


		void processMaterial(const aiScene* pScene);
		void processAnimation(const aiScene* pScene, Model*& model);

	public:
		FBXLoader(){};
		~FBXLoader(){};

		void Initialize(Render::D3DRenderer* renderer );

		Render::D3DRenderer* renderer = nullptr;
		bool ImportModel(Render::Model* model, std::wstring modelPath);
		std::wstring aiToWstring(const aiString& targetString);
	};


}

