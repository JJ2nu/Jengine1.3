#pragma once
//#pragma comment(lib, "DirectXTK.lib")
//#include <directxtk/SimpleMath.h>

#include <vector>
#include <random>
#include <queue>
#include <stack>
using namespace DirectX::SimpleMath;



enum class LocationShape
{
	SPHERE,
	CUBE,
	CYLINDER,
	CONE,
	TORUS,
	SURFACE

};

struct InstanceData {
	Matrix World;
	Vector4 AnimParams; // x:current frame, y:width tiles, z:height tiles
	Vector4 Color;
};

class ParticleVertex
{
public:
	Vector4 position;
	Vector3 normal;
	Vector3 tangent;
	Vector3 bitangent;
	Vector2 tex;

	//Vertex(float x, float y, float z) : position(x, y, z,0) {}
	ParticleVertex() {};
	ParticleVertex(Vector4 position) : position(position) {}
	ParticleVertex(Vector4 position, Vector3 normal, Vector3 tangent, Vector3 bitangent, Vector2 tex) : position(position), normal(normal), tangent(tangent), bitangent(bitangent), tex(tex) {}
	ParticleVertex(Vector4 position, Vector3 normal, Vector3 tangent, Vector2 tex) : position(position), normal(normal), tangent(tangent), bitangent(Vector3(0.f, 0.f, 0.f)), tex(tex) {}
};

class Particle
{
public:
	Vector3 position;
	Vector3 velocity;
	Vector4 color;
	// opacity, size, lifetime, age
	Vector2 scale = Vector2(100.f,100.f );
	float opacity = 1.f;
	float lifetime = 5.f;
	float age = 0.f;
	
	Vector3 m_startVelocity = Vector3{ 0,0,0 };
	Vector4 m_startColor = Vector4{ 1,1,1,1 };
	Vector4 m_endColor = Vector4{ 0.5f,0.5f,0.5f,1 };
	Vector2 m_startScale = Vector2{ 3.f,3.f };
	Vector2 m_endScale = Vector2{ 5.f,5.f };
	float m_startOpacity = 0.5f;
	float m_endOpacity = 0.0f;
	float m_lifetime = 5.f;
	
	
	
	bool active = true;

	size_t index;

	//billboarding
	float zorder = 0.f;
	Matrix scaleMat = Matrix::Identity;
	Matrix rotationMat = Matrix::Identity;
	Matrix translationMat = Matrix::Identity;
	Matrix world = Matrix::Identity;
	Matrix viewMat = Matrix::Identity;
	float rotateAngle = 0.f;
	Vector3 axis = Vector3::Zero;
	Vector3* mainCam;
	Vector3* cameraUp;

	class ParticleEmitter* parentEmitter = nullptr;
	Matrix parentWorld = Matrix::Identity;
	//sprite animation
	Vector4 frameinfo = Vector4(1,1,0,1);
	float _animTimer = 0.f;
	float _animDuration = 1.f / 24.f;
	bool _bIsLoop = true;

	void Update(float deltaTime);

};
class ParticleEmitter 
{
public:
	bool m_Active = true;
	virtual void Initialize(size_t maxParticles = 1000000, float emissionRate = 5.f, Vector3 position = Vector3{ 0,20,0 });
	void InitializeParticle(int idx);
	void InitializeParticle(Particle* particle);
	virtual void Update(float deltaTime);
	virtual void UpdateParticles(float deltaTime, int idx);
	virtual Vector3 LocateShape() = 0;
	void ViewSpaceSort();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_defaultTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_alphaTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalTexture;

	std::vector<Particle> m_particlePool;  // 角力 颇萍努 单捞磐 历厘
	size_t m_activeCount = 0;
	std::stack<size_t> m_inactiveIndices;

    size_t m_maxParticles = 1000000;
    float m_emissionRate = 50.f;
    float m_emissionThreshold = 0.f;
	Vector3 m_emitterPosition = Vector3{ 0,20,0 };
	Quaternion m_emitterRotation = Quaternion::Identity;
	Vector3* mainCam;
	Vector3* cameraUp;

	Matrix viewMat = Matrix::Identity;
	Matrix m_rotationMat = Matrix::Identity;
	Matrix m_translationMat = Matrix::Identity;
	Matrix m_worldMat = Matrix::Identity;
	Vector4 m_startFrame = { 1,1,0,1 };
	float m_speed = 10;
	Vector3 m_startVelocity = Vector3{0,0,0};
	Vector4 m_startColor = Vector4{1,1,1,1};
	Vector4 m_endColor = Vector4{0.5f,0.5f,0.5f,1};
	Vector2 m_startScale = Vector2{ 3.f,3.f };
	Vector2 m_endScale =   Vector2{5.f,5.f};
	float m_startOpacity = 0.5f;
	float m_endOpacity = 0.0f;
	float m_lifetime = 5.f;
	Vector3 axis = Vector3::Zero;
	float _animDuration = 1 / 24.f;
	class ParticleSystem* parentSys;


	//particle attributes
	std::vector<Vector3> position;
	std::vector<Vector3> velocity;
	std::vector<Vector4> color;
	std::vector<Vector2> scale ;
	std::vector<float> opacity;
	std::vector<float> lifetime;
	std::vector<float> age;
	std::vector<Vector4> startColor;
	std::vector<Vector4> endColor ;
	std::vector<Vector2> startScale ;
	std::vector<Vector2> endScale ;
	std::vector<float> startOpacity;
	std::vector<float> endOpacity;

	std::vector<Vector4> frameinfo;
	std::vector<float> animTimer;
	bool bIsLoop = true;
	
	std::vector<Matrix> scaleMat;
	std::vector<Matrix> rotationMat;
	std::vector<Matrix> translationMat;
	std::vector<Matrix> worldMat;
	std::vector<float> zorder;
	float rotateAngle = 0.f;

	void SwapVectors(int i, int j);



	std::vector<Vector4> agebuf;


	std::random_device m_randomizer;
	std::mt19937 m_randomGenerator;
	std::uniform_real_distribution<float> m_randomRange0to1;
	std::function<float()> m_randomVal;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceMat;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_instanceMatSRV;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceAnim;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_instanceAnimSRV;
	
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceColor;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_instanceColorSRV;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceAge;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_instanceAgeSRV;

};
class SphereEmitter : public ParticleEmitter
{
public:
	float radius = 5.f;
	void SetLocateRadius(float param) { radius = param; }
	Vector3 LocateShape() override;
};
class CubeEmitter : public ParticleEmitter
{
public:
	Vector3 scale = Vector3(10, 10, 10);
	void SetLocateScale(Vector3 param) { scale = param; }
	Vector3 LocateShape() override;
};
class CylinderEmitter : public ParticleEmitter
{
public:
	float radius = 5.f;
	float height = 10.f;
	void SetLocateRadius(float param) { radius = param; }
	void SetLocateHeight(float param) { height = param; }
	Vector3 LocateShape() override;
};
class ConeEmitter : public ParticleEmitter
{
public:
	float height = 50.f;
	float angle = DirectX::XM_PIDIV4;
	void SetLocateHeight(float param) { height = param; }
	void SetLocateAngle(float param) { angle = param; }
	Vector3 LocateShape() override;
};
class TorusEmitter : public ParticleEmitter
{
public:
	float innerRadius = 500.f;
	float outerRadius = 1000.f;
	bool isDisc = false;
	void SetLocateInnerRadius(float param) { innerRadius = param; }
	void SetLocateOuterRadius(float param) { outerRadius = param; }
	void SetLocateIfDisc(bool param) { isDisc = param; }
	Vector3 LocateShape() override;
};
class SurfaceEmitter : public ParticleEmitter
{
public:
	SurfaceEmitter(std::vector<Vector3> _vertices)
	{
		vertices = _vertices;
	}
	void Initialize(size_t maxParticles = 100000, float emissionRate = 5.f, Vector3 position = Vector3{ 0,20,0 }) override;
	class Render::Model* targetModel = nullptr;
	float randomOffset = 1.f;
	std::vector<Vector3> vertices;
	std::uniform_int_distribution<int> m_randomRangeVertexIndex;
	std::function<int()> m_randomIndex;
	Vector3 LocateShape() override;
	void Update(float deltaTime) override;
	Vector3 modelScale = { 1,1,1 };
};
class ParticleSystem
{
public:
	bool Initialize();
	void Update(float deltaTime);

	ParticleEmitter* CreateEmitter(LocationShape locationshape , const std::vector<Vector3>& vertices = std::vector<Vector3>());
	void EnableEmitter(size_t index);


	void RemoveEmitter(size_t index);

	std::vector<ParticleEmitter*> m_emitters;
	//void SetParticleVS(Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexshader);
	//void SetParticlePS(Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelshader);
	//void SetParticleIL(Microsoft::WRL::ComPtr<ID3D11InputLayout> inputlayout);
	//void SetParticleBlendState(Microsoft::WRL::ComPtr<ID3D11BlendState> blendstate);
	//Microsoft::WRL::ComPtr<ID3D11VertexShader>  SetParticleVS();
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> SetParticlePS();
	//Microsoft::WRL::ComPtr<ID3D11InputLayout> SetParticleIL();
	//Microsoft::WRL::ComPtr<ID3D11BlendState> SetParticleBlendState();

	Vector3* mainCam;
	Vector3* cameraUp;

	Matrix viewMat = Matrix::Identity;

	std::vector<ParticleVertex> vertices;
	std::vector<UINT> indices;
	UINT _vertexBufferStride;
	UINT _vertexBufferOffset;
	Microsoft::WRL::ComPtr<ID3D11Buffer> _vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> _indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;       // ComPtr肺 包府
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;         // ComPtr肺 包府
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;         // ComPtr肺 包府
	Microsoft::WRL::ComPtr<ID3D11Buffer> _spriteAnimConstantBuffer;

};
