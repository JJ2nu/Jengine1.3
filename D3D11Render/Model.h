#pragma once
#include <map>
using namespace  DirectX::SimpleMath;
namespace Render
{
	struct MatrixPallete;
	class Model;
	struct SkeletalInfo;

	struct BoneRef
	{
		std::wstring nodeName;
		Matrix* nodeWorldMatrixPtr = nullptr;
		UINT boneIdx;
	};
	struct BoneInfo
	{
		SkeletalInfo* m_pSkeletalInfo = nullptr;
		Matrix RelativeTransform;
		Matrix OffsetMatrix;
		std::wstring BoneName;
		std::wstring ParentBoneName;
		int NumChildren = 0;
		//void Set(const aiNode* pNode)
		//{
		//	std::string tempname = pNode->mName.C_Str();
		//	BoneName = std::wstring::assign(tempname.begin(), tempname.end());
		//	RelativeTransform = Matrix(&pNode->mTransformation.a1).Transpose();
		//	NumChildren = pNode->mNumChildren;
		//}

	};
	class Vertex
	{
	public:
		Vector4 position;
		Vector3 normal;
		Vector3 tangent;
		Vector3 bitangent;
		Vector2 tex;

		//Vertex(float x, float y, float z) : position(x, y, z,0) {}
		Vertex() {};
		Vertex(Vector4 position) : position(position) {}
		Vertex(Vector4 position, Vector3 normal, Vector3 tangent, Vector3 bitangent, Vector2 tex) : position(position), normal(normal), tangent(tangent), bitangent(bitangent), tex(tex) {		}
		Vertex(Vector4 position, Vector3 normal, Vector3 tangent, Vector2 tex) : position(position), normal(normal), tangent(tangent), bitangent(Vector3(0.f, 0.f, 0.f)), tex(tex) {}

	};
	class BoneWeightVertex : public Vertex
	{
	public:
		BoneWeightVertex(){} ;
		int BlendIndices[4] = {};
		float BlendWeight[4] = {};
		void AddBoneData(int boneIndex, float weight)
		{
			assert(BlendWeight[0] == 0.0f ||
				BlendWeight[1] == 0.0f ||
				BlendWeight[2] == 0.0f ||
				BlendWeight[3] == 0.0f);
			for (int i = 0; i < 4; i++)
			{
				if (BlendWeight[i] == 0.0f)
				{
					BlendIndices[i] = boneIndex;
					BlendWeight[i] = weight;
					return;
				}

			}
		}
	};
	class Mesh
	{
	public:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer = nullptr;
		UINT m_VertexBufferStride = sizeof(Vertex);
		UINT m_VertexBufferOffset = 0;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer = nullptr;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		int textureIdx = -1;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pShadowVertexShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout = nullptr;
		Matrix* World = nullptr;
		bool bUseAlphaMap = false;
		Mesh() {};
		Mesh(std::vector<Vertex> _vertices, std::vector<unsigned int> _indices) :
			vertices(_vertices), indices(_indices) {}
	};
	class SkeletalMesh : public Mesh
	{
	public:
		std::vector<BoneWeightVertex> m_BoneVertices;
		std::vector<BoneRef> m_BoneReferences;

		MatrixPallete* m_pMatrixPalletePtr = nullptr;
		SkeletalInfo* m_pSkeletalInfo = nullptr;
		SkeletalMesh()=default;
		SkeletalMesh(std::vector<BoneWeightVertex> _vertices, std::vector<unsigned int> _indices)
		{
			indices = std::move(_indices);
			m_BoneVertices = std::move(_vertices);
		}
		void UpdateMatrixPalette();
	};
	struct Texture
	{
		bool bUsePBR = false;
		std::wstring m_diffusePath;
		std::wstring m_normalPath;
		std::wstring m_specularPath;
		std::wstring m_emissivePath;
		std::wstring m_opacityPath;
		std::wstring m_metalnessPath;
		std::wstring m_roughnessPath;
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> m_diffuseRV;
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> m_normalRV;
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> m_specularRV;
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> m_emissiveRV;
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> m_opacityRV;
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> m_metalnessRV;
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> m_roughnessRV;

	};
	struct NodeAnim
	{
		std::wstring	nodeName;

		std::vector<double> time;
		std::vector<Vector3> scaleKey;
		std::vector<Quaternion> rotationKey;
		std::vector<Vector3> translationKey;
		void Evaluate(double time, Vector3& s, Quaternion& r, Vector3& t);
	};
	struct SkeletalInfo
	{
		std::vector<BoneInfo*> bones;
		std::map<std::wstring, UINT> BoneMappingTable;
		std::map<std::wstring, UINT> MeshMappingTable;
		BoneInfo* GetBoneInfoByIndex(UINT idx) const
		{
			return bones[idx];
		}
		UINT GetBoneIndexByName(std::wstring name) const
		{
			return BoneMappingTable.find(name)->second;
		}

	};
	class Node
	{
	public:
		std::wstring m_NodeName;
		Matrix LocalTransform;
		Matrix WorldTransform;
		Matrix ParentWorldTransform = Matrix::Identity;

		Node* parentNode = nullptr;
		Model* model = nullptr;
		std::vector<Node*> childNodes;
		UINT nChildNodes = 0;
		std::vector<UINT> meshes;

		NodeAnim* nodeanim = nullptr;
		double progressAnimTime = 0.f;

		void Update(float deltatime);
		void Render(int windowIdx);
		Node* FindNode(std::wstring name);
	};
	class Animation
	{
	public:
		std::wstring name;
		double m_Duration = 0.f;
		double m_TPS = 0.f;
		double m_totalTime = 0.f;

		std::vector<NodeAnim> nodeChannels;

	};
	class Model
	{
	public:

		Matrix* objectMatrix = nullptr;
		Node* RootNode = nullptr;
		std::vector<Mesh*> m_meshes;
		std::vector<Texture> m_materials;
		std::vector<Animation> m_animations;

		std::vector<SkeletalMesh*> m_skeletalMeshes;
		SkeletalInfo* m_skeletalInfo;

		bool hasBones = true;
		bool isStaticMesh = false;
		bool usePBR = false;
		void Update(float deltatime);
		void Render(int windowIdx);

		UINT curAnimationIdx = 0;
		double curTPS = 0.f;
		double progressAnimTime = 0.f;
		float currentTime = 0.f;

		Node* FindNodeByName(std::wstring name);

	};
}

