#include "pch.h"
#include "../D3D11Render/D3D11Render.h"
#include "Base.h"

#include "../D3D11Render/FBXLoader.h"

using namespace DirectX::SimpleMath;

void Engine::Object::Base::Update(float deltaTime)
{
	defferedflag = false;
	Matrix m_Translation = DirectX::XMMatrixTranslation(Location.x, Location.y, Location.z);
	Matrix m_Rotation = DirectX::XMMatrixRotationX(Rotation.x)
		* DirectX::XMMatrixRotationY(Rotation.y)
		* DirectX::XMMatrixRotationZ(Rotation.z);
	Matrix m_Scale = DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);

	m_Relative = m_Scale * m_Rotation * m_Translation;

	if (m_pParent)
		m_World = m_Relative * m_pParent->m_World;
	else
		m_World = m_Relative;
	model->objectMatrix = &m_World;
	model->Update(deltaTime);


}

void Engine::Object::Base::Render(int windowIdx)
{
	if (defferedflag)
	{
		m_pWorld->m_Renderer->DeferredTraverse(model->RootNode, model);
	}
	else
	{
		m_pWorld->m_Renderer->RenderTraverse(windowIdx, model->RootNode, model);
	}
}

void Engine::Object::Base::Shade(int windowIdx)
{
	m_pWorld->m_Renderer->ShadeTraverse(windowIdx, model->RootNode, model);

}


void Engine::Object::Base::CreateShaders()
{
	if(model->hasBones)
	{
		for (auto& mesh : model->m_skeletalMeshes)
		{

			D3D_SHADER_MACRO macros[] = {"Skinning","1",  NULL,NULL };
			m_pWorld->m_Renderer->CreatePixelShader(pixelShaderPath, mesh);
			m_pWorld->m_Renderer->CreateVertexShaderandInputLayout(vertexShaderPath, mesh, macros);
			m_pWorld->m_Renderer->CreateShadowVertexShader(shadowVertexShaderPath, mesh, macros);
			
		}
	}
	else
	{
		for (auto& mesh : model->m_meshes)
		{
			D3D_SHADER_MACRO macros[] = { NULL,NULL };
			m_pWorld->m_Renderer->CreatePixelShader(pixelShaderPath, mesh);
			m_pWorld->m_Renderer->CreateVertexShaderandInputLayout(vertexShaderPath, mesh, macros);
			m_pWorld->m_Renderer->CreateShadowVertexShader(shadowVertexShaderPath, mesh, macros);
		}
	}




}

void Engine::Object::Base::CreateMaterials()
{

	//m_pWorld->m_Renderer->CreateMaterial(materialPaths[0], mesh,0);
	//m_pWorld->m_Renderer->CreateMaterial(materialPaths[1], mesh,1);
	//m_pWorld->m_Renderer->CreateMaterial(materialPaths[2], mesh,2);
}

void Engine::Object::Base::CreateBuffers()
{
	if (model->hasBones)
	{
		for (auto& mesh : model->m_skeletalMeshes)
		{
			{

				m_pWorld->m_Renderer->CreateVertexBuffer(mesh );
				m_pWorld->m_Renderer->CreateIndexBuffer(*mesh);
			}
		}
	}
	else
	{
		for (auto& mesh : model->m_meshes)
		{
			{
				m_pWorld->m_Renderer->CreateVertexBuffer(mesh);
				m_pWorld->m_Renderer->CreateIndexBuffer(*mesh);
			}
		}

	}

}

void Engine::Object::Base::ImportModel()
{
	m_pWorld->m_Loader->ImportModel(model, modelPath);
}
