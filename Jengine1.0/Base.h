#pragma once

#include "Engine.h"
#include "../D3D11Render/Model.h"

namespace Render
{
	struct Vertex;
}
namespace Engine::Object
{
	class Base //: public std::enable_shared_from_this<Base>
	{
	public:
		Base() = default;

		Base(const Base& other) = default;
		Base(Base&& other) noexcept = default;
		Base& operator=(const Base& other) = default;
		Base& operator=(Base&& other) noexcept = default;
		virtual ~Base() = default;

		virtual void Update(float deltaTime);

		bool defferedflag = false;
		virtual void Render(int windowIdx);
		void Shade(int windowIdx);



		// refactoring needed
		DirectX::XMFLOAT3 Location = DirectX::XMFLOAT3(0,0,0);
		DirectX::XMFLOAT3 Rotation = DirectX::XMFLOAT3(0,0,0);
		DirectX::XMFLOAT3 Scale = DirectX::XMFLOAT3(1,1,1);
		float renderOrder = 0.f;


		DirectX::SimpleMath::Matrix m_World = DirectX::XMMatrixIdentity();
		DirectX::SimpleMath::Matrix m_Relative = DirectX::XMMatrixIdentity();
		Engine::Object::Base* m_pParent = nullptr;;



		Engine::EngineCore* m_pWorld = nullptr;;



		Render::Model* model;
		std::vector<std::wstring > materialPaths;
		std::wstring vertexShaderPath = L"../Shaders/SkinVertexShader.hlsl";
		std::wstring shadowVertexShaderPath = L"../Shaders/ShadowSkinVertexShader.hlsl";
		std::wstring pixelShaderPath = L"../Shaders/SkinPixelShader.cso";

		std::wstring modelPath;

		void CreateShaders();
		void CreateMaterials();
		void CreateBuffers();

		//FBX
		void ImportModel();

	};
}
