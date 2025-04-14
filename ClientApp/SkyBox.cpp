#include "../Jengine1.0/pch.h"
#include "SkyBox.h"

void Client::SkyBox::Initialize(Engine::EngineCore* World,std::wstring _modelPath)
{
	m_pWorld = World;
	modelPath = _modelPath;
	model = new Render::Model();
	model->isStaticMesh = false;
	model->hasBones = false;
	ImportModel();
	vertexShaderPath = L"../Shaders/SkyBoxVs.hlsl";
	pixelShaderPath = L"../Shaders/SkyBoxPs.cso";
	Scale = { 20,20,20 };
	CreateShaders();
	CreateBuffers();
}

void Client::SkyBox::Update(float deltaTime)
{
	//Location = m_pWorld->m_Renderer->mainCam.Eye;
	Base::Update(deltaTime);
}

void Client::SkyBox::Render(int windowIdx)
{
	Base::Render(windowIdx);
}

void Client::SkyBox::DeferredRender(int windowIdx)
{
	m_pWorld->m_Renderer->SetSkyboxPS(model->m_meshes[0]->m_pPixelShader);
	m_pWorld->m_Renderer->isSkybox = true;
	m_pWorld->m_Renderer->DeferredTraverse(model->RootNode, model);
	m_pWorld->m_Renderer->isSkybox = false;
}
