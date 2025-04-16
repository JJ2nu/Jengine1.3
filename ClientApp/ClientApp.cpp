#include "ClientApp.h"
#include "SkyBox.h"
#include <algorithm>
#include <complex>
#include <imgui.h>
#include <memory>

Client::ClientApp::ClientApp(HINSTANCE hInstance)
	:EngineCore(hInstance)
{
	
}

Client::ClientApp::~ClientApp()
{
}


bool Client::ClientApp::Initialize(UINT Width, UINT Height)
{
	__super::Initialize(Width, Height);
	skybox = new SkyBox();
	skybox->Initialize(this, L"../Resource/skybox.fbx");


	//auto ob1 = CreateObject(L"../Resource/SkinningTest2.fbx");
	auto ob1 = CreateObject(L"../Resource/SkinningTest2.fbx");
	ob1->model->hasBones = false;
	ob1->model->isStaticMesh = true;

	ObjectAwake(ob1);
	ob1->Location = { -400,200,800 };
	//ob1->Rotation = { 0,89.5f,0 };

	auto ob3 = CreateObject(L"../Resource/Ground.fbx");
	ob3->model->isStaticMesh = true;
	ob3->model->hasBones = false;
	ob3->model->usePBR = true;
	//ob3->Scale = { 2.f,1.f,2.f };
	ObjectAwake(ob3);
	//for (int i = 0; i < 1; i++)
	//{
	//	m_Renderer->AddBillboard({ 0 + (i / 4) * 20.f ,200,500 + (i % 4) * 20.f }, { 100,100 },
	//		L"../Resource/btest.png",
	//		L"../Resource/btest.png",
	//		L"");
	//	m_Renderer->m_billboards[i]->_frameinfo.frameCnt = Vector4(1, 1, 0, 1);
	//	m_Renderer->m_billboards[i]->_bIsLoop = true;
	//	m_Renderer->m_billboards[i]->spinClockWise = i % 2;
	//	m_Renderer->m_billboards[i]->_animDuration = 1 / 24.f;
	//	m_Renderer->m_billboards[i]->mainCam = &m_Renderer->mainCam;
	//	m_Renderer->m_billboards[i]->axis = Vector3(1, 1, 0);
	//	m_Renderer->m_billboards[i]->scale = Vector2(100.f, 100.f);
	//}

	m_Renderer->CreateParticleSystem();
	m_Renderer->m_particleSystems[0]->CreateEmitter(LocationShape::SPHERE);
	m_Renderer->CreateEmitterInstanceBuffer(m_Renderer->m_particleSystems[0]->m_emitters[0]);
	m_Renderer->SetParticleTexture(0, 0, L"../Resource/defaultSmoke.jpg",
		L"../Resource/defaultSmoke.jpg", L"../Resource/defaultSmokeNormal.jpg");
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_emitterPosition = Vector3(0, 120, 350);
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_emissionRate = 2000.f;
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startScale = Vector2(5, 5);
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_endScale = Vector2(5, 5);
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startOpacity = 0.07f;
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_endOpacity = 0.f;
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_lifetime = 5.f;

	//static_cast<CylinderEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->radius = 100.f;
	//static_cast<CylinderEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->height = 200.f;
	//static_cast<CubeEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->scale = Vector3(100, 50, 200);
	//static_cast<SphereEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->radius = 1.f;


	//std::vector<Vector3> meshVertices;
	//if (false == ob1->model->isStaticMesh)
	//{
	//	for (auto& mesh : ob1->model->m_meshes)
	//	{
	//		for (auto& vertex : mesh->vertices)
	//		{
	//			meshVertices.push_back(Vector3(vertex.position.x, vertex.position.y, vertex.position.z));
	//		}
	//	}
	//}
	//else
	//{
	//	for (auto mesh : ob1->model->m_meshes)
	//	{
	//		for (auto vertex : mesh->vertices)
	//		{
	//			meshVertices.push_back(Vector3(vertex.position.x, vertex.position.y, vertex.position.z));
	//		}
	//	}
	//}
	//m_Renderer->CreateParticleSystem();
 //	m_Renderer->m_particleSystems[0]->CreateEmitter(LocationShape::SURFACE, meshVertices);
	//m_Renderer->SetParticleTexture(0, 0, L"../Resource/defaultFire.jpg",
	//	L"../Resource/defaultFire.jpg", L"../Resource/defaultFireNormal.jpg");
	//static_cast<SurfaceEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->targetModel = ob1->model;
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_emissionRate = 1000.f;
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startScale = Vector2(5, 5);
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_endScale = Vector2(5, 5);
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startVelocity = Vector3(0, 1, 0);
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startOpacity = 0.07f;
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_endOpacity = 0.f;
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_lifetime = 5.f;




	return true;
}


void Client::ClientApp::Update(float deltaTime)
{

	CameraMove();

	skybox->Update(deltaTime);

	for(auto obj : objectList)
	{
		obj->Update(deltaTime);
	}

	for(auto  object : objectList)
	{
		object->renderOrder = (object->m_World * object->m_pWorld->m_Renderer->m_View * object->m_pWorld->m_Renderer->m_Projection).Translation().z;
	}

	std::sort(renderList.begin(), renderList.end(),
		[]( const Engine::Object::Base* ob1,  const Engine::Object::Base* ob2)-> bool
		{
			return ob1->renderOrder > ob2->renderOrder;
		});
	for (auto billboard : m_Renderer->m_billboards)
	{
		billboard->Update(m_Timer.DeltaTime());
	}
	if (!m_Renderer->m_particleSystems.empty())
		m_Renderer->UpdateParticleSystem(deltaTime);
	Engine::EngineCore::Update(deltaTime);

}

void Client::ClientApp::Render(int windowIdx)
{
	m_Renderer->BeginShade(windowIdx);
	for (auto obj : renderList)
	{
		obj->Shade(windowIdx);
	}


	if (windowIdx == 0)
	{
		if (m_Renderer->deferredflag)
		{
			m_Renderer->BeginDeferred();
			skybox->defferedflag = true;
			skybox->DeferredRender(windowIdx);
			for (auto obj : renderList)
			{
				obj->defferedflag = true;
				obj->Render(windowIdx);
			}
			m_Renderer->QuadRender();
		}
		else
		{
			m_Renderer->BeginRender(windowIdx);
			skybox->Render(windowIdx);
			for (auto obj : renderList)
			{
				obj->Render(windowIdx);
			}
		}

		m_Renderer->RenderBillboards(0);
		if(!m_Renderer->m_particleSystems.empty())
			m_Renderer->RenderPaticleSystem();
		m_Renderer->PostProcess();
	}

	if (windowIdx == 1)
	{
		m_Renderer->BeginRender(windowIdx);
		m_Renderer->ImguiRender();
	}

	m_Renderer->EndRender(windowIdx);

}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Client::ClientApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(EngineCore::imguiWnd, message, wParam, lParam))
		return true;

	return __super::WndProc(hWnd, message, wParam, lParam);
}


Engine::Object::Base* Client::ClientApp::CreateObject(std::wstring modelPath)
{
		auto ob1 = new Engine::Object::Base();
		ob1->m_pWorld = this;
		ob1->modelPath = modelPath;
		objectList.push_back(ob1);
		renderList.push_back(ob1);
		ob1->model = new Render::Model();



		return ob1;
}

void Client::ClientApp::ObjectAwake(Engine::Object::Base* ob1)
{
	ob1->ImportModel();
	if (ob1->model->usePBR)
	{
		ob1->vertexShaderPath = L"../Shaders/PBRVertexShader.hlsl";
		ob1->pixelShaderPath = L"../Shaders/PBRPixelShader.cso";
	}
	ob1->CreateShaders();
	ob1->CreateBuffers();
}

void Client::ClientApp::CameraMove()
{
	if (GetForegroundWindow() == EngineCore::m_hWnd)
	{

		if (m_input->IsMouseButtonClicked(0))
		{
			deltaX += m_input->m_currmouseState.lX * 0.001f * cameraspeed;
			deltaY += m_input->m_currmouseState.lY * 0.001f * cameraspeed;

		}

		// 피치 제한: -90도 ~ 90도
		//deltaY = std::clamp(deltaY, -DirectX::XM_PIDIV2, DirectX::XM_PIDIV2);

		// 회전 행렬 계산
		Matrix rotationMatrix = Matrix::CreateFromYawPitchRoll(deltaX, deltaY, 0.0f);

		// look 벡터 갱신
		Vector3 forward = Vector3::Transform(DirectX::SimpleMath::Vector3(0.0f, 0.0f, 1.0f), rotationMatrix);

		m_Renderer->mainCam.To = DirectX::SimpleMath::Vector3(forward.x, forward.y, forward.z);
		m_Renderer->mainCam.To.Normalize();


		Vector3 Right = Vector3::Transform(DirectX::SimpleMath::Vector3(1.0f, 0.0f, 0.0f), rotationMatrix);
		Right.Normalize();


		Vector3 Up = Vector3::Transform(DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f), rotationMatrix);
		Up.Normalize();


		Vector3 newLocation = Vector3(0, 0, 0);
		if (m_input->IsKeyPressed(DIK_UPARROW) || m_input->IsKeyHold(DIK_UPARROW)) newLocation.y += 10.f;
		if (m_input->IsKeyPressed(DIK_DOWNARROW) || m_input->IsKeyHold(DIK_DOWNARROW))newLocation.y -=  10.f;
		if (m_input->IsKeyPressed(DIK_LEFTARROW) || m_input->IsKeyHold(DIK_LEFTARROW)) newLocation.x -=  10.f;
		if (m_input->IsKeyPressed(DIK_RIGHTARROW) || m_input->IsKeyHold(DIK_RIGHTARROW))newLocation.x +=  10.f;
		if (m_input->IsKeyPressed(DIK_C) || m_input->IsKeyHold(DIK_C))newLocation.z += 10.f;
		if (m_input->IsKeyPressed(DIK_V) || m_input->IsKeyHold(DIK_V))newLocation.z -= 10.f;
		objectList[0]->Location.x += newLocation.x;
		objectList[0]->Location.y += newLocation.y;
		objectList[0]->Location.z += newLocation.z;




		if (m_input->IsKeyPressed(DIK_W) || m_input->IsKeyHold(DIK_W)) m_Renderer->mainCam.Eye += m_Renderer->mainCam.To * cameraspeed;
		if (m_input->IsKeyPressed(DIK_S) || m_input->IsKeyHold(DIK_S)) m_Renderer->mainCam.Eye -= m_Renderer->mainCam.To * cameraspeed;
		if (m_input->IsKeyPressed(DIK_A) || m_input->IsKeyHold(DIK_A)) m_Renderer->mainCam.Eye -= Right * cameraspeed;
		if (m_input->IsKeyPressed(DIK_D) || m_input->IsKeyHold(DIK_D)) m_Renderer->mainCam.Eye += Right * cameraspeed;
		if (m_input->IsKeyPressed(DIK_Q) || m_input->IsKeyHold(DIK_Q)) m_Renderer->mainCam.Eye += Up * cameraspeed;
		if (m_input->IsKeyPressed(DIK_E) || m_input->IsKeyHold(DIK_E)) m_Renderer->mainCam.Eye -= Up * cameraspeed;

		if (m_input->IsKeyPressed(DIK_R) || m_input->IsKeyHold(DIK_R)) cameraspeed += 0.1f;
		if (m_input->IsKeyPressed(DIK_T) || m_input->IsKeyHold(DIK_T)) cameraspeed -= 0.1f;


		if (m_input->IsKeyPressed(DIK_I) || m_input->IsKeyHold(DIK_I)) Engine::Time::timeScale = 0.1f;
		if (m_input->IsKeyPressed(DIK_O) || m_input->IsKeyHold(DIK_O)) Engine::Time::timeScale = 1.f;
		
		//m_Renderer->cameraFov -= m_input->GetMouseScroll() * 0.001f;


		if (m_input->IsKeyPressed(DIK_J) || m_input->IsKeyHold(DIK_J)) Engine::Time::timeScale += 0.1f;
		if (m_input->IsKeyPressed(DIK_K) || m_input->IsKeyHold(DIK_K)) Engine::Time::timeScale -= 0.1f;
	}
}
