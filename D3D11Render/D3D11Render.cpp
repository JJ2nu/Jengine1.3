#include "pch.h"
#include <wincodec.h>

#include <algorithm>
#include <Directxtk/DDSTextureLoader.h>
#include <Directxtk/WICTextureLoader.h>
#include "D3D11Render.h"

#include <complex.h>

#include "RenderHelper.h"
#include "Model.h"
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace Engine::Object
{
	class Base;
}

using namespace  DirectX::SimpleMath;
Render::D3DRenderer::D3DRenderer(std::vector<window> _windows) : windows(_windows)
{
}

Render::D3DRenderer::~D3DRenderer()
{
	UninitScene();
	UninitD3D();
	UninitImGUI();

}
bool Render::D3DRenderer::Initialize()
{
	HR_T(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(m_pFactory.GetAddressOf())));
	CoInitialize(nullptr);
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(m_pImagingFactory.GetAddressOf()));
	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HR_T(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, nullptr,
		0, D3D11_SDK_VERSION, m_pDevice.GetAddressOf(), nullptr,
		m_pDeviceContext.GetAddressOf()));

	for (auto window : windows)
	{
		if (!InitD3D(window))
			return false;
	}

	if (!InitScene())
		return false;
	if (!InitImGUI())
		return false;




	CreateMaterial(L"../Resource/iblEnvHDR.dds", m_SkyboxEnvSRV);
	m_pDeviceContext->PSSetShaderResources(5, 1, m_SkyboxEnvSRV.GetAddressOf());

	CreateMaterial(L"../Resource/iblDiffuseHDR.dds", m_SkyboxIrridianceSRV);
	m_pDeviceContext->PSSetShaderResources(8, 1, m_SkyboxIrridianceSRV.GetAddressOf());

	CreateMaterial(L"../Resource/iblBrdf.dds", m_SkyboxLUTBrdfSRV);
	m_pDeviceContext->PSSetShaderResources(9, 1, m_SkyboxLUTBrdfSRV.GetAddressOf());

	CreateMaterial(L"../Resource/iblSpecularHDR.dds", m_SkyboxSpecularSRV);

	m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerLinear.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(1, 1, defaultSampler.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(2, 1, spBRDF_Sampler.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(3, 1, m_pSamplerComparison.GetAddressOf());


	m_ptestlight.lb[0].lightDir = Vector4(0, -1, -1, 1);
	m_ptestlight.lb[1].lightDir = Vector4(0, 1, 0, 1);
	m_ptestlight.lb[2].lightDir = Vector4(1, 0, 0, 1);
	m_ptestlight.lb[3].lightDir = Vector4(-1, 0, 0, 1);





	D3D11_RASTERIZER_DESC RSDesc;
	RSDesc.FillMode = D3D11_FILL_SOLID;           // 평범하게 렌더링
	RSDesc.CullMode = D3D11_CULL_BACK;         // 컬모드 하지 않음
	RSDesc.FrontCounterClockwise = FALSE;        // 시계방향이 뒷면임 CCW
	RSDesc.DepthBias = 0;                      //깊이 바이어스 값 0
	RSDesc.DepthBiasClamp = 0;
	RSDesc.SlopeScaledDepthBias = 0;
	RSDesc.DepthClipEnable = FALSE;            // 깊이 클리핑 없음
	RSDesc.ScissorEnable = FALSE;             // 시저 테스트 하지 않음
	RSDesc.MultisampleEnable = FALSE;          // 멀티 샘플링 하지 않음
	RSDesc.AntialiasedLineEnable = FALSE;         // 라인 안티앨리어싱 없음

	m_pDevice->CreateRasterizerState(&RSDesc, m_pBackCullmodeRS.GetAddressOf());








	return true;
}
void Render::D3DRenderer::Update(float deltaTime)
{
	cameraFov = std::clamp(cameraFov, 0.01f, DirectX::XM_PI/4);
	m_View = DirectX::XMMatrixLookToLH(mainCam.Eye, mainCam.To, mainCam.Up);
	m_Projection = DirectX::XMMatrixPerspectiveFovLH(cameraFov, windows[0].m_Width / (FLOAT)windows[0].m_Height, nearPlane + 0.5f, 10 * (farPlane + 0.01f));
}
void Render::D3DRenderer::BeginRender(int windowIdx)
{
	const float clear_color_with_alpha[4] = { m_ClearColor.x , m_ClearColor.y , m_ClearColor.z, m_ClearColor.w };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView[windowIdx].Get(), clear_color_with_alpha);
	m_pDeviceContext->ClearRenderTargetView(m_PostProcessRTV.Get(), clear_color_with_alpha);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if (windowIdx == 0)
	{
		if (bloomflag)
			m_pDeviceContext->OMSetRenderTargets(1, m_PostProcessRTV.GetAddressOf(), m_pDepthStencilView.Get());
		else
			m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView[0].GetAddressOf(), m_pDepthStencilView.Get());

	}
	else m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView[windowIdx].GetAddressOf(), nullptr);
	m_pDeviceContext->RSSetViewports(1, &viewports[windowIdx]);

	if (windowIdx != 0) return;

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());

	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(1, 1, m_LightBuffer.GetAddressOf());


	m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState.Get(), blendFactor, 0xffffffff);

	m_pDeviceContext->UpdateSubresource(m_pShadowConstantBuffer.Get(), 0, nullptr, &shadowTB, 0, 0);
	m_pDeviceContext->VSSetConstantBuffers(2, 1, m_pShadowConstantBuffer.GetAddressOf());

}
void Render::D3DRenderer::Render(int windowIdx, const Mesh* mesh, const Node* node, const Model* model)
{
	if (windowIdx == 1)
	{
		ImguiRender();
		return;
	}

	ConstantBuffer cb1;
	cb1.mWorld = DirectX::XMMatrixTranspose(node->WorldTransform);
	cb1.mView = DirectX::XMMatrixTranspose(m_View);
	cb1.mProjection = DirectX::XMMatrixTranspose(m_Projection);
	cb1.EyePos = Vector4(mainCam.Eye.x, mainCam.Eye.y, mainCam.Eye.z, 0);

	m_pDeviceContext->VSSetConstantBuffers(1, 1, m_MatrixPallete.GetAddressOf());

	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb1, 0, 0);
	m_pDeviceContext->UpdateSubresource(m_LightBuffer.Get(), 0, nullptr, &m_ptestlight, 0, 0);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	m_pDeviceContext->VSSetShader(mesh->m_pVertexShader.Get(), nullptr, 0);
	m_pDeviceContext->PSSetShader(mesh->m_pPixelShader.Get(), nullptr, 0);
	m_pDeviceContext->PSSetShaderResources(0, 1, model->m_materials[mesh->textureIdx].m_diffuseRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(1, 1, model->m_materials[mesh->textureIdx].m_normalRV.GetAddressOf());

	if (model->usePBR)
	{
		m_pDeviceContext->PSSetShaderResources(2, 1, m_SkyboxSpecularSRV.GetAddressOf());
	}
	else
		m_pDeviceContext->PSSetShaderResources(2, 1, model->m_materials[mesh->textureIdx].m_specularRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(3, 1, model->m_materials[mesh->textureIdx].m_emissiveRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(4, 1, model->m_materials[mesh->textureIdx].m_opacityRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(6, 1, model->m_materials[mesh->textureIdx].m_metalnessRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(7, 1, model->m_materials[mesh->textureIdx].m_roughnessRV.GetAddressOf());

	m_pDeviceContext->PSSetShaderResources(10, 1, m_ShadowMapSRV.GetAddressOf());

	m_pDeviceContext->IASetInputLayout(mesh->m_pInputLayout.Get());
	m_pDeviceContext->IASetVertexBuffers(0, 1, mesh->m_pVertexBuffer.GetAddressOf(), &mesh->m_VertexBufferStride, &mesh->m_VertexBufferOffset);
	m_pDeviceContext->IASetIndexBuffer(mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


	m_pDeviceContext->DrawIndexed(mesh->indices.size(), 0, 0);

}
void Render::D3DRenderer::Render(int windowIdx, const SkeletalMesh* mesh, const Node* node, const Model* model)
{
	if (windowIdx == 1)
	{
		ImguiRender();
		return;
	}

	m_pDeviceContext->VSSetConstantBuffers(1, 1, m_MatrixPallete.GetAddressOf());
	ConstantBuffer cb1;
	cb1.mWorld = DirectX::XMMatrixTranspose(node->WorldTransform);
	cb1.mView = DirectX::XMMatrixTranspose(m_View);
	cb1.mProjection = DirectX::XMMatrixTranspose(m_Projection);
	cb1.EyePos = Vector4(mainCam.Eye.x, mainCam.Eye.y, mainCam.Eye.z, 0);


	m_pDeviceContext->UpdateSubresource(m_MatrixPallete.Get(), 0, nullptr, m_pMatrixPallete, 0, 0);
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb1, 0, 0);
	m_pDeviceContext->UpdateSubresource(m_LightBuffer.Get(), 0, nullptr, &m_ptestlight, 0, 0);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	m_pDeviceContext->VSSetShader(mesh->m_pVertexShader.Get(), nullptr, 0);
	m_pDeviceContext->PSSetShader(mesh->m_pPixelShader.Get(), nullptr, 0);
	m_pDeviceContext->PSSetShaderResources(0, 1, model->m_materials[mesh->textureIdx].m_diffuseRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(1, 1, model->m_materials[mesh->textureIdx].m_normalRV.GetAddressOf());


	if (model->usePBR)
	{
		m_pDeviceContext->PSSetShaderResources(2, 1, m_SkyboxSpecularSRV.GetAddressOf());
	}
	else
		m_pDeviceContext->PSSetShaderResources(2, 1, model->m_materials[mesh->textureIdx].m_specularRV.GetAddressOf());

	m_pDeviceContext->PSSetShaderResources(3, 1, model->m_materials[mesh->textureIdx].m_emissiveRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(4, 1, model->m_materials[mesh->textureIdx].m_opacityRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(6, 1, model->m_materials[mesh->textureIdx].m_metalnessRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(7, 1, model->m_materials[mesh->textureIdx].m_roughnessRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(10, 1, m_ShadowMapSRV.GetAddressOf());

	m_pDeviceContext->IASetInputLayout(mesh->m_pInputLayout.Get());
	m_pDeviceContext->IASetVertexBuffers(0, 1, mesh->m_pVertexBuffer.GetAddressOf(), &mesh->m_VertexBufferStride, &mesh->m_VertexBufferOffset);
	m_pDeviceContext->IASetIndexBuffer(mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


	m_pDeviceContext->DrawIndexed(mesh->indices.size(), 0, 0);

}
void Render::D3DRenderer::BeginShade(int windowIdx)
{
	m_pDeviceContext->OMSetDepthStencilState(m_pZOnDSS.Get(), 1);
	if (windowIdx == 1) return;
	m_pDeviceContext->OMSetRenderTargets(0, NULL, m_ShadowMapDSV.Get());
	m_pDeviceContext->ClearDepthStencilView(m_ShadowMapDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	shadowTB.ShadowProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, m_shadowViewport.Width / (FLOAT)m_shadowViewport.Height, shadowNearPlane, shadowFarPlane);
	mainCam.To.Normalize();
	Vector3 forward = m_View.Forward();
	m_ShadowLookAt = mainCam.Eye + forward * m_ShadowForwardDistFromCamera;
	Vector3 shadowDir = Vector3(m_ptestlight.lb[0].lightDir.x, m_ptestlight.lb[0].lightDir.y, m_ptestlight.lb[0].lightDir.z);
	shadowDir.Normalize();
	m_ShadowPos = m_ShadowLookAt + (-shadowDir * m_ShadowUpDistFromCamera);
	shadowTB.ShadowView = DirectX::XMMatrixLookAtLH(m_ShadowPos, m_ShadowLookAt, Vector3(0.f, 1.f, 0.f));



	shadowTB.ShadowProjection = shadowTB.ShadowProjection.Transpose();
	shadowTB.ShadowView = shadowTB.ShadowView.Transpose();
	m_pDeviceContext->UpdateSubresource(m_pShadowConstantBuffer.Get(), 0, nullptr, &shadowTB, 0, 0);
	m_pDeviceContext->VSSetConstantBuffers(2, 1, m_pShadowConstantBuffer.GetAddressOf());

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->PSSetShader(NULL, NULL, 0);

	m_pDeviceContext->RSSetViewports(1, &m_shadowViewport);
}
void Render::D3DRenderer::Shade(int windowIdx, const Mesh* mesh, const Node* node, const Model* model)
{
	if (windowIdx == 1) return;

	ConstantBuffer cb1;
	cb1.mWorld = DirectX::XMMatrixTranspose(node->WorldTransform);
	cb1.mView = DirectX::XMMatrixTranspose(m_View);
	cb1.mProjection = DirectX::XMMatrixTranspose(m_Projection);
	cb1.EyePos = Vector4(mainCam.Eye.x, mainCam.Eye.y, mainCam.Eye.z, 0);
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb1, 0, 0);

	m_pDeviceContext->VSSetShader(mesh->m_pShadowVertexShader.Get(), nullptr, 0);
	m_pDeviceContext->IASetInputLayout(mesh->m_pInputLayout.Get());
	m_pDeviceContext->IASetVertexBuffers(0, 1, mesh->m_pVertexBuffer.GetAddressOf(), &mesh->m_VertexBufferStride, &mesh->m_VertexBufferOffset);
	m_pDeviceContext->IASetIndexBuffer(mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->DrawIndexed(mesh->indices.size(), 0, 0);
}
void Render::D3DRenderer::Shade(int windowIdx, const SkeletalMesh* mesh, const Node* node, const Model* model)
{
	if (windowIdx == 1) return;

	ConstantBuffer cb1;
	cb1.mWorld = DirectX::XMMatrixTranspose(node->WorldTransform);
	cb1.mView = DirectX::XMMatrixTranspose(m_View);
	cb1.mProjection = DirectX::XMMatrixTranspose(m_Projection);
	cb1.EyePos = Vector4(mainCam.Eye.x, mainCam.Eye.y, mainCam.Eye.z, 0);
	m_pDeviceContext->UpdateSubresource(m_MatrixPallete.Get(), 0, nullptr, m_pMatrixPallete, 0, 0);
	m_pDeviceContext->VSSetConstantBuffers(2, 1, m_MatrixPallete.GetAddressOf());

	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb1, 0, 0);
	m_pDeviceContext->VSSetShader(mesh->m_pShadowVertexShader.Get(), nullptr, 0);
	m_pDeviceContext->IASetInputLayout(mesh->m_pInputLayout.Get());
	m_pDeviceContext->IASetVertexBuffers(0, 1, mesh->m_pVertexBuffer.GetAddressOf(), &mesh->m_VertexBufferStride, &mesh->m_VertexBufferOffset);
	m_pDeviceContext->IASetIndexBuffer(mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->DrawIndexed(mesh->indices.size(), 0, 0);
}
void Render::D3DRenderer::EndRender(int windowIdx)
{
	m_pSwapChains[windowIdx].Get()->Present(0, 0);

}
void Render::D3DRenderer::RenderTraverse(int windowIdx, Node* node, Model* model)
{

	for (auto meshIdx : node->meshes)
	{
		if (model->hasBones)
		{
			Render(windowIdx, model->m_skeletalMeshes[meshIdx], node, model);

		}
		else
		{
			Render(windowIdx, model->m_meshes[meshIdx], node, model);
		}
	}

	for (auto childNode : node->childNodes)
	{
		RenderTraverse(windowIdx, childNode, model);
	}

}
void Render::D3DRenderer::ShadeTraverse(int windowIdx, Node* node, Model* model)
{
	for (auto meshIdx : node->meshes)
	{
		if (model->hasBones)
		{
			Shade(windowIdx, model->m_skeletalMeshes[meshIdx], node, model);

		}
		else
		{
			Shade(windowIdx, model->m_meshes[meshIdx], node, model);
		}
	}

	for (auto childNode : node->childNodes)
	{
		ShadeTraverse(windowIdx, childNode, model);
	}



}
void Render::D3DRenderer::ImguiRender()
{

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	{

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::SetWindowSize(ImVec2(windows[1].m_Width, windows[1].m_Height / 2));
		ImVec2 vec = ImGui::GetWindowSize();

		GetWindowRect(*(windows[0].m_hwnd), &currect);
		if (currect != prevrect)
		{
			SetWindowPos(*(windows[1].m_hwnd), *(windows[0].m_hwnd), currect.right, currect.top, vec.x, vec.y * 2, 0);
			prevrect = currect;
		}

		//ImGui::Text("Forward/Back:  W,S");
		//ImGui::Text("Left/Right:    A,D");
		//ImGui::Text("Up/Down:       Q,E");
		//ImGui::Text("View control:  LeftClick & Move");
		//ImGui::Text("Zoom In/Out:   Mouse Scroll");

		//ImGui::ColorEdit4("Backbuffer", reinterpret_cast<float*>(&m_ClearColor), 0);
		ImGui::Checkbox("Use Deferred", &deferredflag);
		ImGui::Checkbox("Use Bloom", &bloomflag);
		ImGui::Checkbox("normalmap", (bool*) & m_ptestlight.mb.ambient.x);
		ImGui::SliderFloat("bloomThreshold", &bloomThreshold, 0.0f, 1.0f);
		ImGui::SliderFloat("bloomintensity", &bloomIntensity, 0.0f, 100.0f);
		ImGui::SliderInt("blurradius", &bloomblur, 1, 54);
		//ImGui::SliderFloat("near", &nearPlane, 0.0f, 1.0f);
		//ImGui::SliderFloat("far", &farPlane, 0.0f, 100000.0f);
		if (!m_billboards.empty())
		{
			
		ImGui::ColorEdit4("blendcolor", (float*)&m_billboards[0]->_frameinfo.blendColor);
		}
		ImGui::SliderFloat4("lightDirection 0", reinterpret_cast<float*>(&m_ptestlight.lb[0].lightDir), -1.0f, 1.0f);

		ImGui::Text("\n");

		ImGui::Text("\n");

		//변수 임시 재활용 ambient -> metalness , diffuse -> roughness
		ImGui::SliderFloat("useIBL ", &m_ptestlight.mb.ambient.y, 0.f, 1.0f);
		
		
		
		ImGui::Text("particle count %d", m_particleSystems[0]->m_emitters[0]->m_activeParticles.size());
		ImGui::SliderFloat("emit rate", &m_particleSystems[0]->m_emitters[0]->m_emissionRate, 1.f, 10000.0f);
		ImGui::SliderFloat3("emit vector", reinterpret_cast<float*>(&m_particleSystems[0]->m_emitters[0]->m_startVelocity), -1.f, 1.f);
		ImGui::ColorEdit3("particle start color", (float*)&m_particleSystems[0]->m_emitters[0]->m_startColor); // Edit 3 floats representing a color	
		ImGui::ColorEdit3("particle end color", (float*)&m_particleSystems[0]->m_emitters[0]->m_endColor); // Edit 3 floats representing a color	
		ImGui::SliderFloat2("particle start scale", reinterpret_cast<float*>(&m_particleSystems[0]->m_emitters[0]->m_startScale), 0.0f, 1000.0f);
		ImGui::SliderFloat2("particle end scale", reinterpret_cast<float*>(&m_particleSystems[0]->m_emitters[0]->m_endScale), 0.0f, 1000.0f);
		ImGui::SliderFloat("particle start opacity", &m_particleSystems[0]->m_emitters[0]->m_startOpacity, 0.f, 1.f);
		ImGui::SliderFloat("particle end opacity", &m_particleSystems[0]->m_emitters[0]->m_endOpacity, 0.f, 1.f);
		ImGui::SliderFloat("particle lifetime", &m_particleSystems[0]->m_emitters[0]->m_lifetime, 1.f, 10.f);
		ImGui::SliderFloat("Torus outer radius", &static_cast<ConeEmitter*>(m_particleSystems[0]->m_emitters[0])->angle, 0.1f, DirectX::XM_PI);



		ImGui::Text("\n");

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::ColorEdit3("clear color", (float*)&m_ClearColor); // Edit 3 floats representing a color	
	}

	{

		// 0:albedo , 1: Normal , 2: Material, 3: Emissive , 4: shadowpos

		ImGui::Begin("textures");
		ImGui::SetWindowPos(ImVec2(0, windows[1].m_Height / 2));
		ImGui::SetWindowSize(ImVec2(windows[1].m_Width, windows[1].m_Height / 2));

	/*	ImGui::Text("Albedo													Normal");
		ImTextureID imgID = (ImTextureID)(uintptr_t)m_GBufferSRVs[0].Get();
		ImGui::Image(imgID, ImVec2(400, 400));
		ImGui::SameLine();
		ImTextureID imgID1 = (ImTextureID)(uintptr_t)m_GBufferSRVs[1].Get();
		ImGui::Image(imgID1, ImVec2(400, 400));

		ImGui::Text("metal/rough/depth										Emissive");
		ImTextureID imgID2 = (ImTextureID)(uintptr_t)m_GBufferSRVs[2].Get();
		ImGui::Image(imgID2, ImVec2(400, 400));
		ImGui::SameLine();
		ImTextureID imgID3 = (ImTextureID)(uintptr_t)m_GBufferSRVs[3].Get();
		ImGui::Image(imgID3, ImVec2(400, 400));

		ImGui::Text("shadowpos												shadowmap");
		ImTextureID imgID4 = (ImTextureID)(uintptr_t)m_GBufferSRVs[4].Get();
		ImGui::Image(imgID4, ImVec2(400, 400));
		ImGui::SameLine();
		ImTextureID imgID5 = (ImTextureID)(uintptr_t)m_ShadowMapSRV.Get();
		ImGui::Image(imgID5, ImVec2(400, 400));*/

		ImTextureID imgID6 = (ImTextureID)(uintptr_t)m_ExtractSRV.Get();
		ImGui::Image(imgID6, ImVec2(400, 400));
		ImGui::SameLine();
		ImTextureID imgID7 = (ImTextureID)(uintptr_t)m_BlurHSRV.Get();
		ImGui::Image(imgID7, ImVec2(400, 400));

		ImTextureID imgID8 = (ImTextureID)(uintptr_t)m_BlurVSRV.Get();
		ImGui::Image(imgID8, ImVec2(400, 400));
		//if (!m_billboards.empty())
		//{
		//	ImGui::SameLine();
		//	ImTextureID imgID9 = (ImTextureID)(uintptr_t)m_billboards[0]->_BillBoardTexture.Get();
		//	ImGui::Image(imgID9, ImVec2(400, 400));
		//}

		ImGui::End();
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}
bool Render::D3DRenderer::InitD3D(window wnd)
{
	HRESULT hr = 0;

	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = *(wnd.m_hwnd);
	swapDesc.Windowed = true;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Width = wnd.m_Width;
	swapDesc.BufferDesc.Height = wnd.m_Height;
	swapDesc.BufferDesc.RefreshRate.Numerator = 165;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	m_pFactory->CreateSwapChain(m_pDevice.Get(), &swapDesc, swapchain.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBufferTexture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pPostResultTexture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	HR_T(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(pBackBufferTexture.GetAddressOf())));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture.Get(), NULL, renderTargetView.GetAddressOf()));

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(wnd.m_Width);
	viewport.Height = static_cast<float>(wnd.m_Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	viewports.push_back(viewport);
	m_pSwapChains.push_back(swapchain);
	m_pRenderTargetView.push_back(renderTargetView);

	return true;
}
void Render::D3DRenderer::UninitD3D()
{
}
bool Render::D3DRenderer::InitImGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsClassic();
	ImGui_ImplWin32_Init(*(windows[1].m_hwnd));
	ImGui_ImplDX11_Init(m_pDevice.Get(), m_pDeviceContext.Get());

	return true;
}
void Render::D3DRenderer::UninitImGUI()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
void Render::D3DRenderer::CreateBloomResources()
{
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = windows[0].m_Width;
	texDesc.Height = windows[0].m_Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	//Microsoft::WRL::ComPtr<ID3D11Texture2D> bloomTexture;
	//HR_T(m_pDevice->CreateTexture2D(&texDesc, nullptr, bloomTexture.GetAddressOf()));
	//HR_T(m_pDevice->CreateRenderTargetView(bloomTexture.Get(), nullptr, m_BloomRTV.GetAddressOf()));
	//HR_T(m_pDevice->CreateShaderResourceView(bloomTexture.Get(), nullptr, m_BloomSRV.GetAddressOf()));

	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID3DBlob> Buffer;

	HR_T(D3DReadFileToBlob(L"../Shaders/BloomExtract.cso", Buffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(Buffer->GetBufferPointer(),
		Buffer->GetBufferSize(), NULL, m_BloomExtractPS.GetAddressOf()));

	HR_T(D3DReadFileToBlob(L"../Shaders/BloomBlur.cso", Buffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(Buffer->GetBufferPointer(),
		Buffer->GetBufferSize(), NULL, m_BloomBlurPS.GetAddressOf()));

	HR_T(D3DReadFileToBlob(L"../Shaders/BloomCombine.cso", Buffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(Buffer->GetBufferPointer(),
		Buffer->GetBufferSize(), NULL, m_BloomCombinePS.GetAddressOf()));
	// 상수 버퍼 생성
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(BloomParams);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_BloomConstantBuffer.GetAddressOf()));

	D3D11_TEXTURE2D_DESC texdesc = {};
	D3D11_RENDER_TARGET_VIEW_DESC rtvdesc = {};
	D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc = {};

	texdesc.Width = windows[0].m_Width;
	texdesc.Height = windows[1].m_Height;
	texdesc.MipLevels = 1;
	texdesc.ArraySize = 1;
	texdesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texdesc.SampleDesc.Count = 1;
	texdesc.Usage = D3D11_USAGE_DEFAULT;
	texdesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texdesc.CPUAccessFlags = 0;
	texdesc.MiscFlags = 0;

	rtvdesc.Format = texdesc.Format;
	rtvdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvdesc.Texture2D.MipSlice = 0;

	srvdesc.Format = texdesc.Format;
	srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvdesc.Texture2D.MostDetailedMip = 0;
	srvdesc.Texture2D.MipLevels = 1;

	HR_T(m_pDevice->CreateTexture2D(&texdesc, nullptr, m_PostProcessTexture.GetAddressOf()));
	HR_T(m_pDevice->CreateRenderTargetView(m_PostProcessTexture.Get(), &rtvdesc, m_PostProcessRTV.GetAddressOf()));
	HR_T(m_pDevice->CreateShaderResourceView(m_PostProcessTexture.Get(), &srvdesc, m_PostProcessSRV.GetAddressOf()));

	HR_T(m_pDevice->CreateTexture2D(&texdesc, nullptr, m_ExtractTexture.GetAddressOf()));
	HR_T(m_pDevice->CreateRenderTargetView(m_ExtractTexture.Get(), &rtvdesc, m_ExtractRTV.GetAddressOf()));
	HR_T(m_pDevice->CreateShaderResourceView(m_ExtractTexture.Get(), &srvdesc, m_ExtractSRV.GetAddressOf()));

	HR_T(m_pDevice->CreateTexture2D(&texdesc, nullptr, m_BlurVTexture.GetAddressOf()));
	HR_T(m_pDevice->CreateRenderTargetView(m_BlurVTexture.Get(), &rtvdesc, m_BlurVRTV.GetAddressOf()));
	HR_T(m_pDevice->CreateShaderResourceView(m_BlurVTexture.Get(), &srvdesc, m_BlurVSRV.GetAddressOf()));

	HR_T(m_pDevice->CreateTexture2D(&texdesc, nullptr, m_BlurHTexture.GetAddressOf()));
	HR_T(m_pDevice->CreateRenderTargetView(m_BlurHTexture.Get(), &rtvdesc, m_BlurHRTV.GetAddressOf()));
	HR_T(m_pDevice->CreateShaderResourceView(m_BlurHTexture.Get(), &srvdesc, m_BlurHSRV.GetAddressOf()));






	}
void Render::D3DRenderer::ApplyBloom()
{
	// Step 1: 밝기 추출 (Thresholding)
	m_pDeviceContext->OMSetRenderTargets(1, m_ExtractRTV.GetAddressOf(), nullptr);
	m_pDeviceContext->PSSetShaderResources(0, 1, m_PostProcessSRV.GetAddressOf());
	m_pDeviceContext->VSSetShader(m_pQuadVS.Get(), nullptr, 0);
	m_pDeviceContext->PSSetShader(m_BloomExtractPS.Get(), nullptr, 0);
	m_pDeviceContext->PSSetSamplers(0, 1, m_pBloomSampler.GetAddressOf());

	BloomParams params = { bloomThreshold, bloomIntensity ,{0,0},bloomblur};
	m_pDeviceContext->UpdateSubresource(m_BloomConstantBuffer.Get(), 0, nullptr, &params, 0, 0);

	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_BloomConstantBuffer.GetAddressOf());

	RenderFullScreenQuad();

	// Step 2: Gaussian Blur (수평/수직 블러)
		Vector2 blurDirection = Vector2(1.0f / windows[0].m_Width, 0);
		params.blurDirection = blurDirection;
		m_pDeviceContext->OMSetRenderTargets(1, m_BlurHRTV.GetAddressOf(), nullptr);
		m_pDeviceContext->PSSetShaderResources(0, 1, m_ExtractSRV.GetAddressOf());
		m_pDeviceContext->UpdateSubresource(m_BloomConstantBuffer.Get(), 0, nullptr, &params, 0, 0);
		m_pDeviceContext->PSSetShader(m_BloomBlurPS.Get(), nullptr, 0);
		RenderFullScreenQuad();
		blurDirection = Vector2(0, 1.0f / windows[0].m_Height);
		params.blurDirection = blurDirection;
		m_pDeviceContext->OMSetRenderTargets(1, m_BlurVRTV.GetAddressOf(), nullptr);
		m_pDeviceContext->PSSetShaderResources(0, 1, m_BlurHSRV.GetAddressOf());
		m_pDeviceContext->UpdateSubresource(m_BloomConstantBuffer.Get(), 0, nullptr, &params, 0, 0);
		m_pDeviceContext->PSSetShader(m_BloomBlurPS.Get(), nullptr, 0);
		RenderFullScreenQuad();

	////// Step 3: 원본 이미지와 합성
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView[0].GetAddressOf(), nullptr);
	m_pDeviceContext->PSSetShaderResources(0, 1, m_BlurVSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(1, 1, m_PostProcessSRV.GetAddressOf());

	m_pDeviceContext->PSSetShader(m_BloomCombinePS.Get(), nullptr, 0);

	RenderFullScreenQuad();
}
void Render::D3DRenderer::RenderFullScreenQuad()
{
	UINT stride = sizeof(QuadVertex);
	UINT offset = 0;


	m_pDeviceContext->IASetInputLayout(m_pQuadInputLayout.Get());

	m_pDeviceContext->IASetVertexBuffers(0, 1, quad->_vertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(quad->_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_pDeviceContext->PSSetShader(m_pQuadPS.Get(), nullptr, 0);

	m_pDeviceContext->DrawIndexed(quad->_indexCount, 0, 0);
}
void Render::D3DRenderer::InitializeBillboard()
{
	D3D11_RASTERIZER_DESC RSDesc;
	RSDesc.FillMode = D3D11_FILL_SOLID;           // 평범하게 렌더링
	RSDesc.CullMode = D3D11_CULL_NONE;         // 컬모드 하지 않음
	RSDesc.FrontCounterClockwise = FALSE;        // 시계방향이 뒷면임 CCW
	RSDesc.DepthBias = 0;                      //깊이 바이어스 값 0
	RSDesc.DepthBiasClamp = 0;
	RSDesc.SlopeScaledDepthBias = 0;
	RSDesc.DepthClipEnable = FALSE;            // 깊이 클리핑 없음
	RSDesc.ScissorEnable = FALSE;             // 시저 테스트 하지 않음
	RSDesc.MultisampleEnable = FALSE;          // 멀티 샘플링 하지 않음
	RSDesc.AntialiasedLineEnable = FALSE;         // 라인 안티앨리어싱 없음

	m_pDevice->CreateRasterizerState(&RSDesc, m_pNoneCullmodeRS.GetAddressOf());

	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBuffer = nullptr;

	hr = CompileShaderFromFile(L"../Shaders/BillBoardVS.hlsl", "main", "vs_5_0", VertexBuffer.GetAddressOf(),nullptr);
	if (FAILED(hr))
		HR_T(D3DReadFileToBlob(L"../Shaders/BillboardVS.hlsl", VertexBuffer.GetAddressOf()));

	HR_T(m_pDevice->CreateVertexShader(VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), NULL, m_pBillboardVS.GetAddressOf()));
	//자동 layout 생성
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
	D3DReflect(VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)pReflector.GetAddressOf());

	D3D11_SHADER_DESC shaderDesc;
	pReflector->GetDesc(&shaderDesc);

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	inputLayoutDesc.reserve(shaderDesc.InputParameters);

	for (size_t i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC* paramDesc = new D3D11_SIGNATURE_PARAMETER_DESC();
		pReflector->GetInputParameterDesc(i, paramDesc);

		D3D11_INPUT_ELEMENT_DESC elemDesc
		{
			.SemanticName = paramDesc->SemanticName,
			.SemanticIndex = paramDesc->SemanticIndex,
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		};
		if (paramDesc->ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
		{
			if (paramDesc->Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_FLOAT;
			else if (paramDesc->Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			else if (paramDesc->Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			else if (paramDesc->Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		else if (paramDesc->ComponentType == D3D_REGISTER_COMPONENT_UINT32)
		{
			if (paramDesc->Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc->Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc->Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc->Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
		}
		else if (paramDesc->ComponentType == D3D_REGISTER_COMPONENT_SINT32)
		{
			if (paramDesc->Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc->Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc->Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc->Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
		}
		inputLayoutDesc.push_back(elemDesc);
	}

	m_pDevice->CreateInputLayout
	(
		inputLayoutDesc.data(), inputLayoutDesc.size(), VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), m_pBillboardInputLayout.GetAddressOf()
	);

	// 픽셀 셰이더 생성
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
	HR_T(D3DReadFileToBlob(L"../Shaders/BillBoardPS.cso", psBlob.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(), NULL, m_pBillboardPS.GetAddressOf()));
}
void Render::D3DRenderer::RenderBillboards(int windowIdx)
{
	if (windowIdx == 1) return;
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetInputLayout(m_pBillboardInputLayout.Get());


	if (bloomflag)
		m_pDeviceContext->OMSetRenderTargets(1, m_PostProcessRTV.GetAddressOf(), m_pDepthStencilView.Get());
	else
		m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView[0].GetAddressOf(), m_pDepthStencilView.Get());


//	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView[0].GetAddressOf(), m_pDepthStencilView.Get());
	m_pDeviceContext->VSSetShader(m_pBillboardVS.Get(), nullptr, 0);
	
	m_pDeviceContext->PSSetShader(m_pBillboardPS.Get(), nullptr, 0);

	m_pDeviceContext->RSSetState(m_pNoneCullmodeRS.Get());
	m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState.Get(), blendFactor, 0xffffffff);
	m_pDeviceContext->OMSetDepthStencilState(m_pZOffDSS.Get(), 1);


	std::sort(m_billboards.begin(), m_billboards.end(),
		[](const BillboardQuad* ob1, const BillboardQuad* ob2)-> bool
		{
			return ob1->zorder > ob2->zorder;
		});
	for (auto billboard : m_billboards)
	{
		ConstantBuffer cb1;
		cb1.mWorld = DirectX::XMMatrixTranspose(billboard->world);
		cb1.mView = DirectX::XMMatrixTranspose(m_View);
		cb1.mProjection = DirectX::XMMatrixTranspose(m_Projection);
		cb1.EyePos = Vector4(mainCam.Eye.x, mainCam.Eye.y, mainCam.Eye.z, 0);
		//billboard->_frameinfo.blendColor = m_billboards[0]->_frameinfo.blendColor;
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb1, 0, 0);
		m_pDeviceContext->UpdateSubresource(billboard->_billboardAnimConstantBuffer.Get(), 0, nullptr, &billboard->_frameinfo, 0, 0);

		m_pDeviceContext->VSSetConstantBuffers(5, 1, billboard->_billboardAnimConstantBuffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(5, 1, billboard->_billboardAnimConstantBuffer.GetAddressOf());

		m_pDeviceContext->IASetVertexBuffers(0, 1, billboard->_vertexBuffer.GetAddressOf(), &billboard->_vertexBufferStride, &billboard->_vertexBufferOffset);
		m_pDeviceContext->IASetIndexBuffer(billboard->_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_pDeviceContext->PSSetShaderResources(17, 1, billboard->_BillBoardTexture.GetAddressOf());
		m_pDeviceContext->PSSetShaderResources(18, 1, billboard->_alphaTexture.GetAddressOf());
		m_pDeviceContext->PSSetShaderResources(19, 1, billboard->_normalTexture.GetAddressOf());
		m_pDeviceContext->DrawIndexed(6, 0, 0);
	}
	m_pDeviceContext->RSSetState(m_pBackCullmodeRS.Get());

}
void Render::D3DRenderer::AddBillboard(Vector3 pos, Vector2 scale, std::wstring path, std::wstring alphapath, std::wstring normalpath)
{
	BillboardQuad* newBillboard = new BillboardQuad();
	newBillboard->position = pos;
	newBillboard->scale = scale;
	newBillboard->viewMat = &m_View;
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(Vertex) * 4;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = newBillboard->vertices.data();
	HR_T(m_pDevice->CreateBuffer(&vbDesc, &vbData, newBillboard->_vertexBuffer.GetAddressOf()));

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * 6;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = newBillboard->indices.data();
	HR_T(m_pDevice->CreateBuffer(&ibDesc, &ibData, newBillboard->_indexBuffer.GetAddressOf()));

	CreateMaterial(path, newBillboard->_BillBoardTexture);
	if (alphapath.length() > 0)
		CreateMaterial(alphapath, newBillboard->_alphaTexture);
	if (normalpath.length() > 0)
		CreateMaterial(normalpath, newBillboard->_normalTexture);
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(BillboardConstantBuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, newBillboard->_billboardAnimConstantBuffer.GetAddressOf()));

	m_billboards.push_back(newBillboard);

}
void Render::D3DRenderer::CreateParticleSystem()
{
	ParticleSystem* newPS = new ParticleSystem();
	newPS->Initialize();
	newPS->_vertexBufferStride = sizeof(ParticleVertex);
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(ParticleVertex) *4;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = newPS->vertices.data();
	HR_T(m_pDevice->CreateBuffer(&vbDesc, &vbData, newPS->_vertexBuffer.GetAddressOf()));


	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * 6;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = newPS->indices.data();
	HR_T(m_pDevice->CreateBuffer(&ibDesc, &ibData, newPS->_indexBuffer.GetAddressOf()));

	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(SpriteAnimCB);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, newPS->_spriteAnimConstantBuffer.GetAddressOf()));
	
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBuffer = nullptr;

	hr = CompileShaderFromFile(L"../Shaders/BillBoardVS.hlsl", "main", "vs_5_0", VertexBuffer.GetAddressOf(), nullptr);
	if (FAILED(hr))
		HR_T(D3DReadFileToBlob(L"../Shaders/BillboardVS.cso", VertexBuffer.GetAddressOf()));

	HR_T(m_pDevice->CreateVertexShader(VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), NULL, newPS->m_vertexShader.GetAddressOf()));

	//자동 layout 생성
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
	D3DReflect(VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)pReflector.GetAddressOf());

	D3D11_SHADER_DESC shaderDesc;
	pReflector->GetDesc(&shaderDesc);

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	inputLayoutDesc.reserve(shaderDesc.InputParameters);

	for (size_t i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC* paramDesc = new D3D11_SIGNATURE_PARAMETER_DESC();
		pReflector->GetInputParameterDesc(i, paramDesc);

		D3D11_INPUT_ELEMENT_DESC elemDesc
		{
			.SemanticName = paramDesc->SemanticName,
			.SemanticIndex = paramDesc->SemanticIndex,
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		};
		if (paramDesc->ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
		{
			if (paramDesc->Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_FLOAT;
			else if (paramDesc->Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			else if (paramDesc->Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			else if (paramDesc->Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		else if (paramDesc->ComponentType == D3D_REGISTER_COMPONENT_UINT32)
		{
			if (paramDesc->Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc->Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc->Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc->Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
		}
		else if (paramDesc->ComponentType == D3D_REGISTER_COMPONENT_SINT32)
		{
			if (paramDesc->Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc->Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc->Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc->Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
		}
		inputLayoutDesc.push_back(elemDesc);
	}

	m_pDevice->CreateInputLayout
	(
		inputLayoutDesc.data(), inputLayoutDesc.size(), VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), newPS->m_inputLayout.GetAddressOf()
	);

	// 픽셀 셰이더 생성
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
	HR_T(D3DReadFileToBlob(L"../Shaders/BillBoardPS.cso", psBlob.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(), NULL, newPS->m_pixelShader.GetAddressOf()));









	m_particleSystems.push_back(newPS);

}
void Render::D3DRenderer::SetParticleTexture(size_t sysIndex, size_t emitIndex , std::wstring defaultpath, std::wstring alphapath, std::wstring normalpath)
{
	CreateMaterial(defaultpath, m_particleSystems[sysIndex]->m_emitters[emitIndex]->m_defaultTexture);

	if (alphapath.length() > 0)
		CreateMaterial(alphapath, m_particleSystems[sysIndex]->m_emitters[emitIndex]->m_alphaTexture);
	if (normalpath.length() > 0)
		CreateMaterial(normalpath, m_particleSystems[sysIndex]->m_emitters[emitIndex]->m_normalTexture);

	
}
void Render::D3DRenderer::UpdateParticleSystem(float deltaTime)
{
	for (auto system : m_particleSystems)
	{
		system->mainCam = mainCam.Eye;
		system->viewMat = &m_View;
		system->Update(deltaTime);
	}
}
void Render::D3DRenderer::RenderPaticleSystem()
{
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (bloomflag)
		m_pDeviceContext->OMSetRenderTargets(1, m_PostProcessRTV.GetAddressOf(), m_pDepthStencilView.Get());
	else
		m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView[0].GetAddressOf(), m_pDepthStencilView.Get());

	m_pDeviceContext->RSSetState(m_pNoneCullmodeRS.Get());
	m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState.Get(), blendFactor, 0xffffffff);
	m_pDeviceContext->OMSetDepthStencilState(m_pZOnDSS.Get(), 1);

	for (auto system : m_particleSystems)
	{
		m_pDeviceContext->IASetInputLayout(system->m_inputLayout.Get());
		m_pDeviceContext->IASetVertexBuffers(0, 1, system->_vertexBuffer.GetAddressOf(), &system->_vertexBufferStride, &system->_vertexBufferOffset);
		m_pDeviceContext->IASetIndexBuffer(system->_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_pDeviceContext->VSSetShader(system->m_vertexShader.Get(), nullptr, 0);
		m_pDeviceContext->PSSetShader(system->m_pixelShader.Get(), nullptr, 0);
		m_pDeviceContext->VSSetConstantBuffers(5, 1, system->_spriteAnimConstantBuffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(5, 1, system->_spriteAnimConstantBuffer.GetAddressOf());
		for (auto emitter : system->m_emitters)
		{
			RenderEmitters(emitter);
		}
	}
	m_pDeviceContext->RSSetState(m_pBackCullmodeRS.Get());

}
void Render::D3DRenderer::RenderEmitters(ParticleEmitter* emitter)
{

	std::sort(emitter->m_activeParticles.begin(), emitter->m_activeParticles.end(),
		[](const Particle* ob1, const Particle* ob2)-> bool
		{
			return ob1->zorder > ob2->zorder;
		});
	ConstantBuffer cb1;

	m_pDeviceContext->PSSetShaderResources(17, 1, emitter->m_defaultTexture.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(18, 1, emitter->m_alphaTexture.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(19, 1, emitter->m_normalTexture.GetAddressOf());

	for (auto particle : emitter->m_activeParticles)
	{
		cb1.mWorld = DirectX::XMMatrixTranspose(particle->world);
		cb1.mView = DirectX::XMMatrixTranspose(m_View);
		cb1.mProjection = DirectX::XMMatrixTranspose(m_Projection);
		cb1.EyePos = Vector4(mainCam.Eye.x, mainCam.Eye.y, mainCam.Eye.z, 0);
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb1, 0, 0);
		m_pDeviceContext->UpdateSubresource(emitter->parentSys->_spriteAnimConstantBuffer.Get(), 0, nullptr, &particle->_frameinfo, 0, 0);
		m_pDeviceContext->DrawIndexed(6, 0, 0);
	}


	//cb1.mView = DirectX::XMMatrixTranspose(m_View);
	//cb1.mProjection = DirectX::XMMatrixTranspose(m_Projection);
	//cb1.EyePos = Vector4(mainCam.Eye.x, mainCam.Eye.y, mainCam.Eye.z, 0);

	//m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb1, 0, 0);

	//for (auto particle : emitter->m_activeParticles)
	//{
	//	cb1.mWorld = DirectX::XMMatrixTranspose(particle->world);
	//	m_pDeviceContext->UpdateSubresource(emitter->parentSys->_spriteAnimConstantBuffer.Get(), 0, nullptr, &particle->_frameinfo, 0, 0);
	//}

	//m_pDeviceContext->DrawIndexedInstanced(6, 파티클개수, 0, 0, 0);


}
void Render::D3DRenderer::PostProcess()
{

	if (bloomflag)
		ApplyBloom();
}
bool Render::D3DRenderer::InitScene()
{

	CreateDepthStencilView();
	CreateConstantBuffer();
	CreateLightBuffer();
	CreateMatrixPallete();
	CreateAlphaBlendState();
	CreateSampler();
	InitializeMatrices();
	CreateShadowMap();
	InitializeBillboard();
	DeferredInitialize();
	CreateBloomResources();
	return true;
}
void Render::D3DRenderer::CreateMeshBuffers(Mesh& mesh)
{
	// 버텍스 버퍼 생성
	Mesh* tempmesh = &mesh;
	CreateVertexBuffer(tempmesh);

	// 인덱스 버퍼 생성
	CreateIndexBuffer(mesh);
}
void Render::D3DRenderer::CreateVertexBuffer(Mesh* mesh)
{

	if (mesh->vertices.empty()) {
		throw std::runtime_error("Vertex data is empty.");
	}

	if (mesh->m_pVertexBuffer) {
		mesh->m_pVertexBuffer.Reset();
	}

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(Vertex) * mesh->vertices.size();
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = mesh->vertices.data();
	HRESULT hr = m_pDevice->CreateBuffer(&vbDesc, &vbData, mesh->m_pVertexBuffer.GetAddressOf());

	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create vertex buffer.");
	}

	mesh->m_VertexBufferStride = sizeof(Vertex);
	mesh->m_VertexBufferOffset = 0;
}
void Render::D3DRenderer::CreateVertexBuffer(SkeletalMesh* skelMesh)
{
	if (skelMesh->m_BoneVertices.empty())
	{
		throw std::runtime_error("Vertex data is empty.");
	}

	if (skelMesh->m_pVertexBuffer)
	{
		skelMesh->m_pVertexBuffer.Reset();
	}

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(BoneWeightVertex) * skelMesh->m_BoneVertices.size();
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = skelMesh->m_BoneVertices.data();
	HRESULT hr = m_pDevice->CreateBuffer(&vbDesc, &vbData, skelMesh->m_pVertexBuffer.GetAddressOf());

	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create vertex buffer.");
	}

	skelMesh->m_VertexBufferStride = sizeof(BoneWeightVertex);
	skelMesh->m_VertexBufferOffset = 0;

}
void Render::D3DRenderer::CreateIndexBuffer(Mesh& mesh)
{
	UINT indexCount = mesh.indices.size();

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * indexCount;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = mesh.indices.data();
	HR_T(m_pDevice->CreateBuffer(&ibDesc, &ibData, mesh.m_pIndexBuffer.GetAddressOf()));
}
void Render::D3DRenderer::CreateVertexShaderandInputLayout(std::wstring shaderPath, Mesh*& mesh, D3D_SHADER_MACRO macros[])
{
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBuffer = nullptr;

	hr = CompileShaderFromFile(shaderPath.c_str(), "main", "vs_5_0", VertexBuffer.GetAddressOf(), macros);
	if (FAILED(hr))
		HR_T(D3DReadFileToBlob(shaderPath.c_str(), VertexBuffer.GetAddressOf()));

	HR_T(m_pDevice->CreateVertexShader(VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), NULL, mesh->m_pVertexShader.GetAddressOf()));
	//자동 layout 생성
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
	D3DReflect(VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)pReflector.GetAddressOf());

	D3D11_SHADER_DESC shaderDesc;
	pReflector->GetDesc(&shaderDesc);

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	inputLayoutDesc.reserve(shaderDesc.InputParameters);

	for (size_t i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pReflector->GetInputParameterDesc(i, &paramDesc);

		D3D11_INPUT_ELEMENT_DESC elemDesc
		{
			.SemanticName = paramDesc.SemanticName,
			.SemanticIndex = paramDesc.SemanticIndex,
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		};
		if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
		{
			if (paramDesc.Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_FLOAT;
			else if (paramDesc.Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			else if (paramDesc.Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			else if (paramDesc.Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
		{
			if (paramDesc.Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
		}
		else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
		{
			if (paramDesc.Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
		}
		inputLayoutDesc.push_back(elemDesc);
	}

	m_pDevice->CreateInputLayout
	(
		inputLayoutDesc.data(), inputLayoutDesc.size(), VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), mesh->m_pInputLayout.GetAddressOf()
	);


}
void Render::D3DRenderer::CreateVertexShaderandInputLayout(std::wstring shaderPath, SkeletalMesh*& mesh, D3D_SHADER_MACRO macros[])
{
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBuffer = nullptr;

	hr = CompileShaderFromFile(shaderPath.c_str(), "main", "vs_5_0", VertexBuffer.GetAddressOf(), macros);
	if (FAILED(hr))
		HR_T(D3DReadFileToBlob(shaderPath.c_str(), VertexBuffer.GetAddressOf()));

	HR_T(m_pDevice->CreateVertexShader(VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), NULL, mesh->m_pVertexShader.GetAddressOf()));

	//자동 layout 생성
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
	D3DReflect(VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)pReflector.GetAddressOf());

	D3D11_SHADER_DESC shaderDesc;
	pReflector->GetDesc(&shaderDesc);

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	inputLayoutDesc.reserve(shaderDesc.InputParameters);

	for (size_t i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pReflector->GetInputParameterDesc(i, &paramDesc);

		D3D11_INPUT_ELEMENT_DESC elemDesc
		{
			.SemanticName = paramDesc.SemanticName,
			.SemanticIndex = paramDesc.SemanticIndex,
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		};
		if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
		{
			if (paramDesc.Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_FLOAT;
			else if (paramDesc.Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			else if (paramDesc.Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			else if (paramDesc.Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
		{
			if (paramDesc.Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
		}
		else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
		{
			if (paramDesc.Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
		}
		inputLayoutDesc.push_back(elemDesc);
	}

	m_pDevice->CreateInputLayout
	(
		inputLayoutDesc.data(), inputLayoutDesc.size(), VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), mesh->m_pInputLayout.GetAddressOf()
	);


}
void Render::D3DRenderer::CreateShadowVertexShader(std::wstring shaderPath, Mesh*& mesh, D3D_SHADER_MACRO macros[])
{
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBuffer = nullptr;

	hr = CompileShaderFromFile(shaderPath.c_str(), "main", "vs_5_0", VertexBuffer.GetAddressOf(), macros);
	if (FAILED(hr))
		HR_T(D3DReadFileToBlob(shaderPath.c_str(), VertexBuffer.GetAddressOf()));

	HR_T(m_pDevice->CreateVertexShader(VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), NULL, mesh->m_pShadowVertexShader.GetAddressOf()));

}
void Render::D3DRenderer::CreateShadowVertexShader(std::wstring shaderPath, SkeletalMesh*& mesh, D3D_SHADER_MACRO macros[])
{
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBuffer = nullptr;

	hr = CompileShaderFromFile(shaderPath.c_str(), "main", "vs_5_0", VertexBuffer.GetAddressOf(), macros);
	if (FAILED(hr))
		HR_T(D3DReadFileToBlob(shaderPath.c_str(), VertexBuffer.GetAddressOf()));

	HR_T(m_pDevice->CreateVertexShader(VertexBuffer->GetBufferPointer(),
		VertexBuffer->GetBufferSize(), NULL, mesh->m_pShadowVertexShader.GetAddressOf()));
}
void Render::D3DRenderer::CreatePixelShader(std::wstring shaderPath, Mesh*& mesh)
{

	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID3DBlob> Buffer;


	HR_T(D3DReadFileToBlob(shaderPath.c_str(), Buffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(Buffer->GetBufferPointer(),
		Buffer->GetBufferSize(), NULL, mesh->m_pPixelShader.GetAddressOf()));

}
void Render::D3DRenderer::CreatePixelShader(std::wstring shaderPath, SkeletalMesh*& mesh)
{

	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID3DBlob> Buffer;

	HR_T(D3DReadFileToBlob(shaderPath.c_str(), Buffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(Buffer->GetBufferPointer(),
		Buffer->GetBufferSize(), NULL, mesh->m_pPixelShader.GetAddressOf()));
}
void Render::D3DRenderer::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_pConstantBuffer.GetAddressOf()));


	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(ShadowTransformBuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_pShadowConstantBuffer.GetAddressOf()));

}
void Render::D3DRenderer::CreateLightBuffer()
{
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(LightBuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_LightBuffer.GetAddressOf()));
}
void Render::D3DRenderer::CreateMatrixPallete()
{
	m_pMatrixPallete = new MatrixPallete();
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(MatrixPallete);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_MatrixPallete.GetAddressOf()));
}
void Render::D3DRenderer::CreateAlphaBlendState()
{
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
	rtBlendDesc.BlendEnable = true;
	rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;

	rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0] = rtBlendDesc;
	HR_T(m_pDevice->CreateBlendState(&blendDesc, m_pAlphaBlendState.GetAddressOf()));

}
void Render::D3DRenderer::CreateDepthStencilView()
{
	//6. 뎊스&스텐실 뷰 생성
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = windows[0].m_Width;
	descDepth.Height = windows[0].m_Height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> textureDepthStencil = nullptr;
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, textureDepthStencil.GetAddressOf()));

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR_T(m_pDevice->CreateDepthStencilView(textureDepthStencil.Get(), &descDSV, m_pDepthStencilView.GetAddressOf()));

	D3D11_DEPTH_STENCIL_DESC depthStateDesc;
	ZeroMemory(&depthStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthStateDesc.DepthEnable = true;
	depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStateDesc.DepthFunc = D3D11_COMPARISON_LESS;//가까운물체만 렌더
	depthStateDesc.StencilEnable = false;
	HR_T(m_pDevice->CreateDepthStencilState(&depthStateDesc, m_pZOnDSS.GetAddressOf()));
	m_pDeviceContext->OMSetDepthStencilState(m_pZOnDSS.Get(), 1);

	D3D11_DEPTH_STENCIL_DESC ddepthStateDesc;
	ZeroMemory(&ddepthStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	ddepthStateDesc.DepthEnable = false;
	ddepthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	ddepthStateDesc.DepthFunc = D3D11_COMPARISON_LESS;//가까운물체만 렌더
	ddepthStateDesc.StencilEnable = false;
	HR_T(m_pDevice->CreateDepthStencilState(&ddepthStateDesc, m_pZOffDSS.GetAddressOf()));



}
void Render::D3DRenderer::CreateSampler()
{

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T(m_pDevice->CreateSamplerState(&sampDesc, m_pSamplerLinear.GetAddressOf()));

	sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T(m_pDevice->CreateSamplerState(&sampDesc, m_pBloomSampler.GetAddressOf()));






	sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	HR_T(m_pDevice->CreateSamplerState(&sampDesc, m_pSamplerComparison.GetAddressOf()));
}
void Render::D3DRenderer::CreateMaterial(std::wstring path, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv)
{
	HRESULT hr = DirectX::CreateWICTextureFromFile(m_pDevice.Get(), path.c_str(), nullptr, srv.GetAddressOf());
	if (FAILED(hr) && path != L"")
	{
		DirectX::TexMetadata metadata;
		DirectX::ScratchImage image;
		HRESULT hr = S_OK;
		hr = LoadFromTGAFile(path.c_str(), &metadata, image);
		if (FAILED(hr))
		{
			HR_T(LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_ALLOW_LARGE_FILES, &metadata, image));
		}
		HR_T(DirectX::CreateShaderResourceView(
			m_pDevice.Get(),
			image.GetImages(),
			image.GetImageCount(),
			metadata,
			srv.GetAddressOf()
		));
	}

}
void Render::D3DRenderer::CreateMaterialFromDDS(std::wstring path, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv)
{
	DirectX::ScratchImage image;

	HR_T(LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_ALLOW_LARGE_FILES, nullptr, image));
	HR_T(CreateShaderResourceView(m_pDevice.Get(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), srv.GetAddressOf()));

}
void Render::D3DRenderer::CreateShadowMap()
{
	m_shadowViewport.TopLeftX = 0;
	m_shadowViewport.TopLeftY = 0;
	m_shadowViewport.Width = 8192;
	m_shadowViewport.Height = 8192;
	m_shadowViewport.MinDepth = 0.0f;
	m_shadowViewport.MaxDepth = 1.0f;

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = (UINT)m_shadowViewport.Width;
	texDesc.Height = (UINT)m_shadowViewport.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	HR_T(m_pDevice->CreateTexture2D(&texDesc, NULL, m_ShadowMap.GetAddressOf()));

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	HR_T(m_pDevice->CreateDepthStencilView(m_ShadowMap.Get(), &descDSV, m_ShadowMapDSV.GetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV = {};
	descSRV.Format = DXGI_FORMAT_R32_FLOAT;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	descSRV.Texture2D.MipLevels = 1;
	HR_T(m_pDevice->CreateShaderResourceView(m_ShadowMap.Get(), &descSRV, m_ShadowMapSRV.GetAddressOf()));



}
void Render::D3DRenderer::DeferredInitialize()
{
	m_GBufferTextures.resize(GBUFSIZE);
	m_GBufferSRVs.resize(GBUFSIZE);
	m_GBufferRTVs.resize(GBUFSIZE);

	D3D11_TEXTURE2D_DESC texdesc = {};
	D3D11_RENDER_TARGET_VIEW_DESC rtvdesc = {};
	D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc = {};

	texdesc.Width = windows[0].m_Width;
	texdesc.Height = windows[1].m_Height;
	texdesc.MipLevels = 1;
	texdesc.ArraySize = 1;
	texdesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texdesc.SampleDesc.Count = 1;
	texdesc.Usage = D3D11_USAGE_DEFAULT;
	texdesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texdesc.CPUAccessFlags = 0;
	texdesc.MiscFlags = 0;
	for (size_t i = 0; i < GBUFSIZE; ++i)
		HR_T(m_pDevice->CreateTexture2D(&texdesc, NULL, m_GBufferTextures[i].GetAddressOf()));

	rtvdesc.Format = texdesc.Format;
	rtvdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvdesc.Texture2D.MipSlice = 0;
	for (size_t i = 0; i < GBUFSIZE; ++i)
		HR_T(m_pDevice->CreateRenderTargetView(m_GBufferTextures[i].Get(), &rtvdesc, m_GBufferRTVs[i].GetAddressOf()));

	srvdesc.Format = texdesc.Format;
	srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvdesc.Texture2D.MostDetailedMip = 0;
	srvdesc.Texture2D.MipLevels = 1;
	for (size_t i = 0; i < GBUFSIZE; ++i)
		HR_T(m_pDevice->CreateShaderResourceView(m_GBufferTextures[i].Get(), &srvdesc, m_GBufferSRVs[i].GetAddressOf()));

	D3D11_BUFFER_DESC bufdesc = {};
	bufdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufdesc.Usage = D3D11_USAGE_DEFAULT;
	bufdesc.CPUAccessFlags = 0;
	bufdesc.ByteWidth = sizeof(Render::DeferredConstantBuffer);

	HR_T(m_pDevice->CreateBuffer(&bufdesc, NULL, m_deferredConstant.GetAddressOf()));


	Microsoft::WRL::ComPtr<ID3DBlob> Buffer;
	HR_T(D3DReadFileToBlob(L"../Shaders/DeferredPBRPS.cso", Buffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(Buffer->GetBufferPointer(),
		Buffer->GetBufferSize(), NULL, m_pDeferredPBRPixelShader.GetAddressOf()));

	//HR_T(D3DReadFileToBlob(L"../Shaders/DeferredSkinPS.cso", Buffer.GetAddressOf()));
	//HR_T(m_pDevice->CreatePixelShader(Buffer->GetBufferPointer(),
	//	Buffer->GetBufferSize(), NULL, m_pDeferredSkinPixelShader.GetAddressOf()));

	//quad
	{

		HRESULT hr = S_OK;
		Microsoft::WRL::ComPtr<ID3DBlob> VertexBuffer = nullptr;

		hr = CompileShaderFromFile(L"../Shaders/QuadVertexShader.hlsl", "main", "vs_5_0", VertexBuffer.GetAddressOf(),NULL);
		if (FAILED(hr))
			HR_T(D3DReadFileToBlob(L"../Shaders/QuadVertexShader.cso", VertexBuffer.GetAddressOf()));

		HR_T(m_pDevice->CreateVertexShader(VertexBuffer->GetBufferPointer(),
			VertexBuffer->GetBufferSize(), NULL, m_pQuadVS.GetAddressOf()));
		//자동 layout 생성
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
		D3DReflect(VertexBuffer->GetBufferPointer(),
			VertexBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)pReflector.GetAddressOf());

		D3D11_SHADER_DESC shaderDesc;
		pReflector->GetDesc(&shaderDesc);

		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
		inputLayoutDesc.reserve(shaderDesc.InputParameters);

		for (size_t i = 0; i < shaderDesc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			pReflector->GetInputParameterDesc(i, &paramDesc);

			D3D11_INPUT_ELEMENT_DESC elemDesc
			{
				.SemanticName = paramDesc.SemanticName,
				.SemanticIndex = paramDesc.SemanticIndex,
				.InputSlot = 0,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0
			};
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				if (paramDesc.Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_FLOAT;
				else if (paramDesc.Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
				else if (paramDesc.Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				else if (paramDesc.Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				if (paramDesc.Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_UINT;
				else if (paramDesc.Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if (paramDesc.Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if (paramDesc.Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				if (paramDesc.Mask == 1) elemDesc.Format = DXGI_FORMAT_R32_SINT;
				else if (paramDesc.Mask <= 3) elemDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if (paramDesc.Mask <= 7) elemDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if (paramDesc.Mask <= 15) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			}
			inputLayoutDesc.push_back(elemDesc);
		}

		m_pDevice->CreateInputLayout
		(
			inputLayoutDesc.data(), inputLayoutDesc.size(), VertexBuffer->GetBufferPointer(),
			VertexBuffer->GetBufferSize(), m_pQuadInputLayout.GetAddressOf()
		);

		Microsoft::WRL::ComPtr<ID3DBlob> Buffer;
		HR_T(D3DReadFileToBlob(L"../Shaders/QuadPixelShader.cso", Buffer.GetAddressOf()));
		HR_T(m_pDevice->CreatePixelShader(Buffer->GetBufferPointer(),
			Buffer->GetBufferSize(), NULL, m_pQuadPS.GetAddressOf()));

	}

	InitializeQuad();

}
void Render::D3DRenderer::BeginDeferred()
{
	m_pDeviceContext->RSSetViewports(1, &viewports[0]);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_pDeviceContext->OMSetRenderTargets(GBUFSIZE, m_GBufferRTVs.data()->GetAddressOf(), m_pDepthStencilView.Get());

	const float clear_color_with_alpha[4] = { m_ClearColor.x , m_ClearColor.y , m_ClearColor.z, m_ClearColor.w };
	for (auto rt : m_GBufferRTVs)
		m_pDeviceContext->ClearRenderTargetView(rt.Get(), clear_color_with_alpha);

	Render::DeferredConstantBuffer db = {};
	db.NearFar.x = 0.01f;
	db.NearFar.y = 100000.f;
	m_pDeviceContext->UpdateSubresource(m_deferredConstant.Get(), 0, nullptr ,&db, 0, 0);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(1, 1, m_LightBuffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(2, 1, m_pShadowConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(4, 1, m_deferredConstant.GetAddressOf());
	m_pDeviceContext->OMSetDepthStencilState(m_pZOnDSS.Get(), 1);
}
void Render::D3DRenderer::DeferredTraverse( Node* node, Model* model)
{
	for (auto meshIdx : node->meshes)
	{
		if (model->hasBones)
		{
			DeferredPass( model->m_skeletalMeshes[meshIdx], node, model);

		}
		else
		{
			DeferredPass( model->m_meshes[meshIdx], node, model);
		}
	}

	for (auto childNode : node->childNodes)
	{
		DeferredTraverse( childNode, model);
	}

}
void Render::D3DRenderer::DeferredPass( const Mesh* mesh, const Node* node, const Model* model)
{
	ConstantBuffer cb1;
	cb1.mWorld = DirectX::XMMatrixTranspose(node->WorldTransform);
	cb1.mView = DirectX::XMMatrixTranspose(m_View);
	cb1.mProjection = DirectX::XMMatrixTranspose(m_Projection);
	cb1.EyePos = Vector4(mainCam.Eye.x, mainCam.Eye.y, mainCam.Eye.z, 0);

	m_pDeviceContext->VSSetConstantBuffers(1, 1, m_MatrixPallete.GetAddressOf());

	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb1, 0, 0);
	m_pDeviceContext->UpdateSubresource(m_LightBuffer.Get(), 0, nullptr, &m_ptestlight, 0, 0);


	m_pDeviceContext->VSSetShader(mesh->m_pVertexShader.Get(), nullptr, 0);

	if (!isSkybox)
		m_pDeviceContext->PSSetShader(m_pDeferredPBRPixelShader.Get(), nullptr, 0);

	m_pDeviceContext->PSSetShaderResources(0, 1, model->m_materials[mesh->textureIdx].m_diffuseRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(1, 1, model->m_materials[mesh->textureIdx].m_normalRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(2, 1, m_SkyboxSpecularSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(3, 1, model->m_materials[mesh->textureIdx].m_emissiveRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(4, 1, model->m_materials[mesh->textureIdx].m_opacityRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(6, 1, model->m_materials[mesh->textureIdx].m_metalnessRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(7, 1, model->m_materials[mesh->textureIdx].m_roughnessRV.GetAddressOf());

	m_pDeviceContext->PSSetShaderResources(10, 1, m_ShadowMapSRV.GetAddressOf());

	m_pDeviceContext->IASetInputLayout(mesh->m_pInputLayout.Get());
	m_pDeviceContext->IASetVertexBuffers(0, 1, mesh->m_pVertexBuffer.GetAddressOf(), &mesh->m_VertexBufferStride, &mesh->m_VertexBufferOffset);
	m_pDeviceContext->IASetIndexBuffer(mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


	m_pDeviceContext->DrawIndexed(mesh->indices.size(), 0, 0);

}
void Render::D3DRenderer::DeferredPass( const SkeletalMesh* mesh, const Node* node, const Model* model)
{


}
void Render::D3DRenderer::InitializeQuad()
{
	quad = new Quad();
	std::vector<QuadVertex> vertices;
	std::vector<unsigned int> indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc, constantBufferDesc = {};
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set the number of vertices in the vertex array.
	quad->_vertexCount = 4;
	vertices.resize(quad->_vertexCount);

	// Set the number of indices in the index array.
	quad->_indexCount = 6;
	indices.resize(quad->_indexCount);


	vertices[0].position = Vector4(-1.0f, 1.0f, 0.0f, 1.0f); // Top left
	vertices[1].position = Vector4(1.0f, 1.0f, 0.0f, 1.0f); // Top right
	vertices[2].position = Vector4(-1.0f, -1.0f, 0.0f, 1.0f); // Bottom left
	vertices[3].position = Vector4(1.0f, -1.0f, 0.0f, 1.0f); // Bottom right

	vertices[0].texture = Vector2(0.0f, 0.0f); // Top left
	vertices[1].texture = Vector2(1.0f, 0.0f); // Top right
	vertices[2].texture = Vector2(0.0f, 1.0f); // Bottom left
	vertices[3].texture = Vector2(1.0f, 1.0f); // Bottom right

	indices = { 0, 1, 2, 2, 1, 3 };
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(QuadVertex) * quad->_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	vertexData.pSysMem = vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, quad->_vertexBuffer.GetAddressOf());
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * quad->_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, quad->_indexBuffer.GetAddressOf());

	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = 0;
	m_pDevice->CreateBuffer(&constantBufferDesc, NULL, quad->_constant.GetAddressOf());

}
void Render::D3DRenderer::QuadRender()
{
	const float clear_color_with_alpha[4] = { m_ClearColor.x , m_ClearColor.y , m_ClearColor.z, m_ClearColor.w };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView[0].Get(), clear_color_with_alpha);
	//m_pDeviceContext->ClearRenderTargetView(m_PostProcessRTV.Get(), clear_color_with_alpha);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if (bloomflag)
		m_pDeviceContext->OMSetRenderTargets(1, m_PostProcessRTV.GetAddressOf(), m_pDepthStencilView.Get());
	else 
		m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView[0].GetAddressOf(), m_pDepthStencilView.Get());
	m_pDeviceContext->RSSetViewports(1, &viewports[0]);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(2, 1, m_pShadowConstantBuffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(4, 1, m_deferredConstant.GetAddressOf());

	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(1, 1, m_LightBuffer.GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(2, 1, m_pShadowConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(4, 1, m_deferredConstant.GetAddressOf());

	m_pDeviceContext->OMSetDepthStencilState(m_pZOffDSS.Get(), 1);


	m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState.Get(), blendFactor, 0xffffffff);



	unsigned int stride;
	unsigned int offset;
	// Set vertex buffer stride and offset.
	stride = sizeof(QuadVertex);
	offset = 0;

	DeferredConstantBuffer db = {};
	DirectX::SimpleMath::Matrix IView = m_View;
	DirectX::SimpleMath::Matrix IProj = m_Projection;
	IView = DirectX::XMMatrixInverse(nullptr, IView);
	IProj = DirectX::XMMatrixInverse(nullptr, IProj);
	db.InverseViewMatrix = DirectX::XMMatrixTranspose(IView);
	db.InverseProjectionMatrix = DirectX::XMMatrixTranspose(IProj);
	m_pDeviceContext->UpdateSubresource(m_deferredConstant.Get(), 0, nullptr, &db, 0, 0);



	ConstantBuffer cb;
	m_pDeviceContext->IASetVertexBuffers(0, 1, quad->_vertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(quad->_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetInputLayout(m_pQuadInputLayout.Get());
	m_pDeviceContext->VSSetShader(m_pQuadVS.Get(), nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pQuadPS.Get(), nullptr, 0);
	m_pDeviceContext->PSSetShaderResources(12, GBUFSIZE, m_GBufferSRVs.data()->GetAddressOf());


	m_pDeviceContext->DrawIndexed(quad->_indexCount, 0, 0);
}
void Render::D3DRenderer::SetSkyboxPS(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps)
{
	m_pDeviceContext->PSSetShader(ps.Get(), nullptr, 0);
	//m_ptestlight.mb.emissive.x = 1.0f;
}
void Render::D3DRenderer::InitializeMatrices()
{
	// 월드, 뷰, 프로젝션 행렬 초기화
	m_World = DirectX::XMMatrixIdentity();
	m_View = DirectX::XMMatrixLookAtLH(mainCam.Eye, mainCam.At, mainCam.Up);
	m_Projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4,
		windows[0].m_Width / static_cast<FLOAT>(windows[0].m_Height), nearPlane, farPlane);
}
void Render::D3DRenderer::UninitScene()
{
}
