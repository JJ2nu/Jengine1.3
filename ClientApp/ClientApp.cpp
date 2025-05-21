#include "ClientApp.h"
#include "SkyBox.h"
#include <algorithm>
#include <complex>
#include "imgui.h"
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
	//skybox = new SkyBox();
	//skybox->Initialize(this, L"../Resource/skybox.fbx");


	//auto ob1 = CreateObject(L"../Resource/SkinningTest2.fbx");
	//auto ob1 = CreateObject(L"../Resource/spaceship.fbx");
	auto ob1 = CreateObject(L"../Resource/Artorias_Sword.fbx");
	ob1->model->hasBones = true;
	ob1->model->isStaticMesh = true;

	ObjectAwake(ob1);
	ob1->Location = { 0, 10, 1000 };


	//auto wall1 = CreateObject(L"../Resource/BigwindowWall_Clear.fbx");
	//wall1->model->hasBones = false;
	//wall1->model->isStaticMesh = true;
	//ObjectAwake(wall1);
	//wall1->Location = { 50, 10, 350 };
	//wall1->Scale = { 0.2f,0.2f,0.2f };
	//auto wall2 = CreateObject(L"../Resource/Wall_Test.fbx");
	//wall2->model->hasBones = false;
	//wall2->model->isStaticMesh = true;
	//ObjectAwake(wall2);
	//wall2->Location = { -50, 10, 350 };
	//ob1->Scale = { 0.2f, 0.2f, 0.2f };
	//ob1->Rotation = { 0,89.5f,0 };

	//auto ob3 = CreateObject(L"../Resource/Ground.fbx");
	//ob3->model->isStaticMesh = true;
	//ob3->model->hasBones = false;
	//ob3->model->usePBR = true;
	////ob3->Scale = { 2.f,1.f,2.f };
	//ObjectAwake(ob3);


	//m_Renderer->CreateParticleSystem();
	//m_Renderer->m_particleSystems[0]->CreateEmitter(LocationShape::TORUS);
	//m_Renderer->CreateEmitterInstanceBuffer(m_Renderer->m_particleSystems[0]->m_emitters[0]);
	//m_Renderer->SetParticleTexture(0, 0, L"../Resource/defaultSmoke.jpg",
	//	L"../Resource/defaultSmoke.jpg", L"");
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_maxParticles = 100;
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_emitterPosition = Vector3(0, 0, 350);
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_emissionRate = 1;
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startScale = Vector2(5, 5);
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_endScale = Vector2(5, 5);
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startOpacity = 0.07f;
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_endOpacity = 0.f;
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_lifetime = 5.f;
	////m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startFrame = { Vector4(8,4,0,32)};
	//static_cast<TorusEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->isDisc = false;
	//static_cast<TorusEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->innerRadius = 100;
	//static_cast<TorusEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->outerRadius = 110;

	//m_Renderer->m_particleSystems[0]->CreateEmitter(LocationShape::SPHERE);
	//m_Renderer->CreateEmitterInstanceBuffer(m_Renderer->m_particleSystems[0]->m_emitters[1]);
	//m_Renderer->SetParticleTexture(0, 1, L"../Resource/defaultSmoke.jpg",
	//	L"../Resource/defaultSmoke.jpg", L"");
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_maxParticles = 100;
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_emitterPosition = Vector3(0, 0, 350);
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_emissionRate = 1;
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_startScale = Vector2(5, 5);
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_endScale = Vector2(5, 5);
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_startOpacity = 0.07f;
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_endOpacity = 0.f;
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_lifetime = 5.f;
	////m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startFrame = { Vector4(8,4,0,32)};
	////static_cast<TorusEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->isDisc = false;
	////static_cast<TorusEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->innerRadius = 100;
	////static_cast<TorusEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->outerRadius = 110;







	std::vector<Vector3> meshVertices;
	std::vector<UINT> meshIndices;
	if (false == ob1->model->isStaticMesh)
	{
		for (auto& mesh : ob1->model->m_meshes)
		{
			for (auto& vertex : mesh->vertices)
			{
				meshVertices.push_back(Vector3(vertex.position.x, vertex.position.y, vertex.position.z));
			}
			for (auto& index : mesh->indices)
			{
				meshIndices.push_back(index);
			}
		}
	}
	else
	{
		for (auto mesh : ob1->model->m_skeletalMeshes)
		{
			for (auto vertex : mesh->m_BoneVertices)
			{
				meshVertices.push_back(Vector3(vertex.position.x, vertex.position.y, vertex.position.z));
			}
			for (auto& index : mesh->indices)
			{
				meshIndices.push_back(index);
			}
		}
	}
	for (int i = 0; i < meshIndices.size();i += 3)
	{
		auto newVector = (meshVertices[meshIndices[i]] + meshVertices[meshIndices[i + 1]] + meshVertices[meshIndices[i + 2]]) * 0.333f;
		auto newVector1 = (meshVertices[meshIndices[i]] + newVector) * 0.5f;
		auto newVector2 = (meshVertices[meshIndices[i+1]] + newVector) * 0.5f;
		auto newVector3 = (meshVertices[meshIndices[i+2]] + newVector) * 0.5f;

		auto newVector4 = (meshVertices[meshIndices[i]] + meshVertices[meshIndices[i+2]] ) * 0.5f;
		auto newVector5 = (meshVertices[meshIndices[i+1]] + meshVertices[meshIndices[i+2]] ) * 0.5f;
		auto newVector6 = (meshVertices[meshIndices[i+1]] + meshVertices[meshIndices[i]] ) * 0.5f;
		meshVertices.push_back(newVector);
		meshVertices.push_back(newVector1);
		meshVertices.push_back(newVector2);
		meshVertices.push_back(newVector3);
		meshVertices.push_back(newVector4);
		meshVertices.push_back(newVector5);
		meshVertices.push_back(newVector6);
	}

	m_Renderer->CreateParticleSystem();
	m_Renderer->m_particleSystems[0]->CreateEmitter(LocationShape::SURFACE, meshVertices);
	m_Renderer->CreateEmitterInstanceBuffer(m_Renderer->m_particleSystems[0]->m_emitters[0]);

	m_Renderer->SetParticleTexture(0, 0, L"../Resource/defaultSmoke.jpg",
		L"../Resource/defaultSmoke.jpg", L"");
	static_cast<SurfaceEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->targetModel = ob1->model;
	static_cast<SurfaceEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->modelScale = { 0.2f,0.2f,0.2f };
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_emissionRate = 1000.f;
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startScale = Vector2(5, 5);
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_endScale = Vector2(5, 5);
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startVelocity = Vector3(0, 1, 0);
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startOpacity = 0.07f;
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_endOpacity = 0.f;
	m_Renderer->m_particleSystems[0]->m_emitters[0]->m_lifetime = 5.f;
	//m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startFrame = { Vector4(8,4,0,32) };

	//m_Renderer->m_particleSystems[0]->CreateEmitter(LocationShape::TORUS);
	//m_Renderer->CreateEmitterInstanceBuffer(m_Renderer->m_particleSystems[0]->m_emitters[1]);
	//m_Renderer->SetParticleTexture(0, 1, L"../Resource/defaultSmoke.jpg",
	//	L"../Resource/defaultSmoke.jpg", L"");
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_maxParticles = 100;
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_emitterPosition = Vector3(0, 10, 350);
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_emissionRate = 1000.f;
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_startScale = Vector2(5, 5);
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_endScale = Vector2(5, 5);
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_startOpacity = 0.07f;
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_endOpacity = 0.f;
	//m_Renderer->m_particleSystems[0]->m_emitters[1]->m_lifetime = 5.f;
	////m_Renderer->m_particleSystems[0]->m_emitters[0]->m_startFrame = { Vector4(8,4,0,32)};
	//static_cast<TorusEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[1])->isDisc = false;
	//static_cast<TorusEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[1])->innerRadius = 100;
	//static_cast<TorusEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[1])->outerRadius = 110;







	return true;
}


void Client::ClientApp::Update(float deltaTime)
{
	CameraMove();
	
	
	if (m_input->IsKeyPressed(DIK_L))
	{
		lerpflag *= -1;
	}
	particleoffset += deltaTime * lerpflag*3000.f;
	if (particleoffset < 1)
		particleoffset = 1.f;
	else if (particleoffset > 5000.f)
		particleoffset = 5000.f;
	static_cast<SurfaceEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->randomOffset = particleoffset;




	//if (m_input->IsKeyPressed(DIK_K))
	//{
	//	emitterrotateflag = (emitterrotateflag == false);
	//	
	//}
	//if (emitterrotateflag)
	//{
	//	emitterRotation.z += deltaTime;
	//	if (emitterRotation.z >= DirectX::XM_PI * 2)
	//	{
	//		emitterRotation.z -= DirectX::XM_PI * 2;
	//	}
	//	static_cast<SurfaceEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->m_emitterRotation = Quaternion::CreateFromYawPitchRoll(emitterRotation);
	//}



	static_cast<SurfaceEmitter*>(m_Renderer->m_particleSystems[0]->m_emitters[0])->modelScale = m_Renderer->modelimguiscale;
	//skybox->Update(deltaTime);

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
	//if (!m_Renderer->m_particleSystems.empty())
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
			//skybox->defferedflag = true;
			//skybox->DeferredRender(windowIdx);
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
			//skybox->Render(windowIdx);
			for (auto obj : renderList)
			{
				obj->Render(windowIdx);
			}
		}

		m_Renderer->RenderBillboards(0);
		//if(!m_Renderer->m_particleSystems.empty())
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
	//if (ob1->model->usePBR)
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
		if (m_input->IsKeyPressed(DIK_UPARROW) || m_input->IsKeyHold(DIK_UPARROW)) newLocation.y += 1.f;
		if (m_input->IsKeyPressed(DIK_DOWNARROW) || m_input->IsKeyHold(DIK_DOWNARROW))newLocation.y -=  1.f;
		if (m_input->IsKeyPressed(DIK_LEFTARROW) || m_input->IsKeyHold(DIK_LEFTARROW)) newLocation.x -=  1.f;
		if (m_input->IsKeyPressed(DIK_RIGHTARROW) || m_input->IsKeyHold(DIK_RIGHTARROW))newLocation.x +=  1.f;
		if (m_input->IsKeyPressed(DIK_C) || m_input->IsKeyHold(DIK_C))newLocation.z += 1.f;
		if (m_input->IsKeyPressed(DIK_V) || m_input->IsKeyHold(DIK_V))newLocation.z -= 1.f;
		objectList[0]->Location.x += newLocation.x;
		objectList[0]->Location.y += newLocation.y;
		objectList[0]->Location.z += newLocation.z;




		if (m_input->IsKeyPressed(DIK_W) || m_input->IsKeyHold(DIK_W)) m_Renderer->mainCam.Eye += m_Renderer->mainCam.To * cameraspeed;
		if (m_input->IsKeyPressed(DIK_S) || m_input->IsKeyHold(DIK_S)) m_Renderer->mainCam.Eye -= m_Renderer->mainCam.To * cameraspeed;
		if (m_input->IsKeyPressed(DIK_A) || m_input->IsKeyHold(DIK_A)) m_Renderer->mainCam.Eye -= Right * cameraspeed;
		if (m_input->IsKeyPressed(DIK_D) || m_input->IsKeyHold(DIK_D)) m_Renderer->mainCam.Eye += Right * cameraspeed;
		if (m_input->IsKeyPressed(DIK_Q) || m_input->IsKeyHold(DIK_Q)) m_Renderer->mainCam.Eye += Up * cameraspeed;
		if (m_input->IsKeyPressed(DIK_E) || m_input->IsKeyHold(DIK_E)) m_Renderer->mainCam.Eye -= Up * cameraspeed;

		if (m_input->IsKeyPressed(DIK_R) || m_input->IsKeyHold(DIK_R)) cameraspeed += 0.01f;
		if (m_input->IsKeyPressed(DIK_T) || m_input->IsKeyHold(DIK_T)) cameraspeed -= 0.01f;


		if (m_input->IsKeyPressed(DIK_I) || m_input->IsKeyHold(DIK_I)) Engine::Time::timeScale = 0.1f;
		if (m_input->IsKeyPressed(DIK_O) || m_input->IsKeyHold(DIK_O)) Engine::Time::timeScale = 1.f;
		
		//m_Renderer->cameraFov -= m_input->GetMouseScroll() * 0.001f;


		if (m_input->IsKeyPressed(DIK_J) || m_input->IsKeyHold(DIK_J)) Engine::Time::timeScale += 0.1f;
		if (m_input->IsKeyPressed(DIK_K) || m_input->IsKeyHold(DIK_K)) Engine::Time::timeScale -= 0.1f;

	


	}
}

void Client::ClientApp::offsetLerp(float delta)
{

}
