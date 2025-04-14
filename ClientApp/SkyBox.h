#pragma once
#include "../Jengine1.0/Base.h"
namespace Client

{
	class SkyBox : public Engine::Object::Base
	{
	public:
		void Initialize(Engine::EngineCore* m_pWorld,std::wstring modelPath);
		void Update(float deltaTime) override;
		void Render(int windowIdx) override;
		void DeferredRender(int windowIdx);
	};
}
