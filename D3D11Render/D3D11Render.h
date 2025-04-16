#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <filesystem>
#include <wincodec.h>
#include <directxtk/SimpleMath.h>

#include "Model.h"
#include "Particles.h"

#define GBUFSIZE 5

using namespace DirectX::SimpleMath;

struct ImDrawList;
namespace Render
{
	class Model;
	struct Mesh;
	struct Vertex;
	struct window
	{
		HWND* m_hwnd;
		float m_Width;
		float m_Height;
	};
	struct Camera
	{
		DirectX::SimpleMath::Vector3 Eye = DirectX::SimpleMath::Vector3(0.0f, 200.0f, -300.0f);
		DirectX::SimpleMath::Vector3 To = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 1.0f);
		DirectX::SimpleMath::Vector3 At = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
		DirectX::SimpleMath::Vector3 Up = DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
	};
	struct ConstantBuffer
	{
		Matrix mWorld;
		Matrix mView;
		Matrix mProjection;
		Vector4 EyePos;
	};
	struct BillboardConstantBuffer
	{
		Vector4 frameCnt;
		Vector4 blendColor = { 1.f,1.f,1.f,0.5f };
	};
	struct ShadowTransformBuffer
	{
		Matrix ShadowView;
		Matrix ShadowProjection;
	};
	struct DeferredConstantBuffer
	{
		Matrix InverseViewMatrix;
		Matrix InverseProjectionMatrix;
		Vector2 NearFar;
		Vector2 Padding{};
	};
	struct Light
	{
		Vector4 lightDir = { 0.f,0.f,1.f,0.f };
		Vector4 ambient = { 0.5f,0.5f,0.5f,0.f };
		Vector4 diffuse = { 1.f,1.f,1.f,0.f };
		Vector4 specular = { 1.f,1.f,1.f,0.f };
		Vector4 emissive = { 1.f,1.f,1.f,0.f };
		Vector4 lightSource = { 0.f,0.f,-2.f,1.f };
		Vector4 lightRadiance = { 1.f,1.f,1.f,1.f };
	};
	struct Material
	{
		Vector4 ambient = { 0.0f,0.0f,0.0f,0.f };
		Vector4 diffuse = { 0.0f,0.0f,0.0f,0.f };
		Vector4 specular = { 1.f,1.f,1.f,0.f };
		Vector4 emissive = { 1.f,1.f,1.f,0.f };
		Vector4 specularExponent = { 32.f,0.f,0.f,0.f };
	};
	struct LightBuffer
	{
		Light lb[4];
		Material mb;
		bool flag1 = true;
		Vector3 pad1;
		bool flag2 = true;
		Vector3 pad2;
	};
	struct MatrixPallete
	{
		Matrix Array[128];
	};
	struct Quad
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> _vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _constant;
		int _vertexCount, _indexCount;
	};
	struct QuadVertex
	{
		Vector4 position;
		Vector2 texture;
	};

	struct BloomParams {
		float threshold;
		float bloomIntensity;
		Vector2 blurDirection;
		int blurRadius;
		float dummy;
		float dummy2;
		float dummy3;
	};
	struct BillboardQuad
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _BillBoardTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _alphaTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _normalTexture;
		std::wstring _alphaTexturePath;
		std::wstring _normalTexturePath;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _billboardAnimConstantBuffer;
		BillboardConstantBuffer _frameinfo;
		float zorder = 0.f;
		int _vertexCount = 4;
		int _indexCount = 6;
		UINT _vertexBufferStride = sizeof(Vertex);
		UINT _vertexBufferOffset = 0;
		Matrix* viewMat = nullptr;

		Matrix scaleMat = Matrix::Identity;
		Matrix rotationMat = Matrix::Identity;
		Matrix translationMat = Matrix::Identity;
		Matrix world = Matrix::Identity;
		float rotateAngle = 0.f;
		bool spinClockWise = false;

		Vector3 position;  // 파티클 위치
		Vector2 scale;      // 빌보드 크기 (너비, 높이)
		Vector3 axis = Vector3::Zero;
		Camera* mainCam = nullptr;


		float _animTimer = 0.f;
		float _animDuration = 1.f / 32.f;
		bool _bIsLoop = false;
		BillboardQuad()
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

			position = Vector3(0, 20, 0);
			scale = Vector2(1, 1);

		}
		void Update(float deltaTime)
		{
			scaleMat = DirectX::SimpleMath::Matrix::CreateScale(scale.x, scale.y, 1);
			translationMat = DirectX::SimpleMath::Matrix::CreateTranslation(position);
			Matrix viewRotInv = *viewMat;
			//billboarding
			{
				if (axis != Vector3::Zero)
				{
					Vector3 toCamera = mainCam->Eye - position;
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
						rotationMat * viewRotInv *
						translationMat;
				}
			}
			//view space sorting
			{
				Vector3 s, t;
				Quaternion q;
				Matrix temp = world * (*viewMat);
				temp.Decompose(s, q, t);
				zorder = t.z;
			}
			// sprite animation
			{
				if (_frameinfo.frameCnt.w > 1)
				{
					_animTimer += deltaTime;
					if (_animTimer >= _animDuration)
					{
						if (_bIsLoop)
						{
							_frameinfo.frameCnt.z = static_cast<int>(_frameinfo.frameCnt.z + 1) % static_cast<int>(_frameinfo.frameCnt.w);
						}
						else
						{
							if (_frameinfo.frameCnt.z < _frameinfo.frameCnt.w)
								_frameinfo.frameCnt.z++;
						}
						_animTimer -= _animDuration;
					}
				}

			}
		}

	};

	class D3DRenderer
	{
	public:
		D3DRenderer(std::vector<window> windows);
		virtual ~D3DRenderer();

		RECT currect = { 0,0,0,0 };
		RECT prevrect = { 0,0,0,0 };


		Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice = nullptr;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
		Microsoft::WRL::ComPtr<IDXGIFactory> m_pFactory = nullptr;
		Microsoft::WRL::ComPtr<IWICImagingFactory> m_pImagingFactory = nullptr;


		std::vector< Microsoft::WRL::ComPtr<IDXGISwapChain>> m_pSwapChains;
		std::vector < Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_pRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pDepthStencilView = nullptr;  // 깊이값 처리를 위한 뎊스스텐실 뷰
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_pAlphaBlendState = nullptr;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pZOnDSS = nullptr;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pZOffDSS = nullptr;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SkyboxEnvSRV = nullptr;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SkyboxIrridianceSRV = nullptr;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SkyboxSpecularSRV = nullptr;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SkyboxLUTBrdfSRV = nullptr;

		D3D11_VIEWPORT m_shadowViewport = {};
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShadowMapSRV = nullptr;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_ShadowMapDSV = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_ShadowMap = nullptr;

		FLOAT blendFactor[4] = { 1.f,1.f,1.f, 0.5f };
		Microsoft::WRL::ComPtr <ID3D11SamplerState> m_pSamplerLinear = nullptr;
		Microsoft::WRL::ComPtr <ID3D11SamplerState> m_pBloomSampler = nullptr;
		Microsoft::WRL::ComPtr <ID3D11SamplerState> m_pSamplerComparison = nullptr;
		Microsoft::WRL::ComPtr <ID3D11SamplerState> spBRDF_Sampler = nullptr;
		Microsoft::WRL::ComPtr <ID3D11SamplerState> defaultSampler = nullptr;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pConstantBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_LightBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_MatrixPallete = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pShadowConstantBuffer = nullptr;
		float TotalTimeElapsed = 0;

		bool isSkybox = false;


		Matrix                m_World;				// 월드좌표계 공간으로 변환을 위한 행렬.
		Matrix                m_View;				// 카메라좌표계 공간으로 변환을 위한 행렬.
		Matrix                m_Projection;			// 단위장치좌표계( Normalized Device Coordinate) 공간으로 변환을 위한 행렬.

		Vector3 m_ShadowLookAt;
		float m_ShadowForwardDistFromCamera = 3500.f;
		float m_ShadowUpDistFromCamera = 13000.f;
		Vector3 m_ShadowPos;



		std::vector<D3D11_VIEWPORT> viewports;



		std::vector<window> windows;

		virtual bool Initialize();
		virtual void Update(float deltaTime);


		virtual void BeginRender(int windowIdx);

		void Render(int windowIdx, const Mesh* mesh, const Node* node, const Model* model);
		void Render(int windowIdx, const SkeletalMesh* mesh, const Node* node, const Model* model);

		void BeginShade(int windowIdx);
		void Shade(int windowIdx, const Mesh* mesh, const Node* node, const Model* model);
		void Shade(int windowIdx, const SkeletalMesh* mesh, const Node* node, const Model* model);
		virtual void EndRender(int windowIdx);

		void RenderTraverse(int windowIdx, Node* node, Model* model);
		void ShadeTraverse(int windowIdx, Node* node, Model* model);


		void ImguiRender();

		//test
		LightBuffer m_ptestlight;
		bool traceObject = true;;

		ShadowTransformBuffer shadowTB;
		MatrixPallete* m_pMatrixPallete;



		Vector4 m_ClearColor = Vector4(0.f, 0.f, 0.f, 1.0f);
		bool m_show_another_window = false;
		bool m_show_demo_window = true;
		float m_f;
		int m_counter;

		float cameraFov = DirectX::XM_PIDIV4;
		float cameraRotateX = 0.0f;
		float cameraRotateY = 0.0f;
		float cameraRotateZ = 0.0f;

		Vector4 ob1Location = { 0,0,200,0 };
		Vector4 ob1Rotation = { 0,0,0,0 };
		Vector4 ob1Scale = { 0.2f,0.2f,0.2f, 0.2f };

		Vector4 ob2Location = { 10,0,20,0 };
		Vector4 ob2Rotation = { 0,0,0,0 };
		Vector4 ob2Scale = { 0.2f,0.2f,0.2f, 0.2f };

		float nearPlane = 0.01f;
		float farPlane = 10000.0f;
		float shadowNearPlane = 7000.f;
		float shadowFarPlane = 20000.0f;

		float skyboxScale = 1.f;

		Camera mainCam;


		void DrawGrid();
		std::vector<Render::Vertex> GridVertices;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> gridvertexshader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> gridpixelshader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> gridpVertexBuffer = nullptr;


		bool InitD3D(window wnd);
		void UninitD3D();

		bool InitScene();
		void CreateMeshBuffers(Mesh& mesh);
		void CreateVertexBuffer(Mesh* mesh);
		void CreateVertexBuffer(SkeletalMesh* mesh);
		void CreateIndexBuffer(Mesh& mesh);
		void CreateVertexShaderandInputLayout(std::wstring shaderPath, Mesh*& mesh, D3D_SHADER_MACRO macros[]);
		void CreateVertexShaderandInputLayout(std::wstring shaderPath, SkeletalMesh*& mesh, D3D_SHADER_MACRO macros[]);
		void CreateShadowVertexShader(std::wstring shaderPath, Mesh*& mesh, D3D_SHADER_MACRO macros[]);
		void CreateShadowVertexShader(std::wstring shaderPath, SkeletalMesh*& mesh, D3D_SHADER_MACRO macros[]);

		void CreatePixelShader(std::wstring shaderPath, Mesh*& mesh);
		void CreatePixelShader(std::wstring shaderPath, SkeletalMesh*& mesh);


		void CreateConstantBuffer();
		void CreateLightBuffer();
		void CreateMatrixPallete();

		void CreateAlphaBlendState();
		void CreateDepthStencilView();
		void CreateSampler();
		void CreateMaterial(std::wstring path, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv);
		void CreateMaterialFromDDS(std::wstring path, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv);
		void CreateShadowMap();

		// 0: depth, 1:albedo , 2: Normal , 3: Material, 4: Emissive , 5: shadowpos
		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_GBufferTextures;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_GBufferSRVs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_GBufferRTVs;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_deferredConstant = nullptr;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pDeferredPBRPixelShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pDeferredSkinPixelShader = nullptr;

		bool deferredflag = false;

		void DeferredInitialize();

		void BeginDeferred();

		void DeferredTraverse(Node* node, Model* model);
		void DeferredPass(const Mesh* mesh, const Node* node, const Model* model);
		void DeferredPass(const SkeletalMesh* mesh, const Node* node, const Model* model);
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pQuadVS = nullptr;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pQuadPS = nullptr;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pQuadInputLayout = nullptr;
		void InitializeQuad();
		void QuadRender();
		Quad* quad;
		void SetSkyboxPS(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps);



		void InitializeMatrices();
		void UninitScene();

		bool InitImGUI();
		void UninitImGUI();


		// Bloom 관련 멤버 변수
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_PostProcessRTV;  // Bloom 렌더 타겟
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_PostProcessSRV;  // Bloom 텍스처
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_PostProcessTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_BloomSRV;  // Bloom 텍스처

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_ExtractRTV;  // Bloom 렌더 타겟
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ExtractSRV;  // Bloom 텍스처
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_ExtractTexture;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_BlurHRTV;  // Bloom 렌더 타겟
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_BlurHSRV;  // Bloom 텍스처
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_BlurHTexture;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_BlurVRTV;  // Bloom 렌더 타겟
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_BlurVSRV;  // Bloom 텍스처
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_BlurVTexture;



		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_BloomExtractPS;  // 밝기 추출 셰이더
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_BloomBlurPS;     // Gaussian Blur 셰이더
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_BloomCombinePS;  // 합성 셰이더
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_BloomConstantBuffer;  // Bloom 상수 버퍼

		float bloomThreshold = 0.25f;       // 밝기 임계값
		float bloomIntensity = 1.5f;       // Bloom 강도
		int bloomblur = 15;       // Bloom 강도
		bool bloomflag = false;

		void CreateBloomResources();       // Bloom 관련 리소스 생성
		void ApplyBloom();                 // Bloom 처리 함수
		void RenderFullScreenQuad();



		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pNoneCullmodeRS;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pBackCullmodeRS;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pBillboardVS;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pBillboardPS;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pBillboardInputLayout;

		std::vector<BillboardQuad*> m_billboards;  // 빌보드 버텍스 데이터
		UINT m_billboardCount = 0;


		void InitializeBillboard();
		void RenderBillboards(int windowIdx);
		void AddBillboard(Vector3 pos, Vector2 scale, std::wstring path, std::wstring alphapath, std::wstring normalpath);

		std::vector<ParticleSystem*> m_particleSystems;
		void CreateParticleSystem();
		void CreateEmitterInstanceBuffer(class ParticleEmitter* _emitter);
		void SetParticleTexture(size_t sysIndex, size_t emitIndex,std::wstring defaultpath, std::wstring alphapath, std::wstring normalpath);
		void UpdateParticleSystem(float deltaTime);
		void RenderPaticleSystem();
		void RenderEmitters(ParticleEmitter* emitter);
		void RenderEmittersInstanced(ParticleEmitter* emitter);

		bool _isParticleInstanced = false;






		void PostProcess();


	};


}

