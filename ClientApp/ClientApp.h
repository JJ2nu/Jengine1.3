#pragma once
#pragma comment(lib, "Jengine1.0.lib")
#include "../Jengine1.0/pch.h"

namespace Client
{
	class SkyBox;

	class ClientApp : public Engine::EngineCore
	{
	public:
		ClientApp(HINSTANCE hInstance);
		~ClientApp();

		virtual bool Initialize(UINT Width, UINT Height);
		void CreateObject();
		virtual void Update(float deltatime);
		virtual void Render(int windowIdx);
		virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		float timeElapsed = 0.f;
		void DrawGrid();

		Engine::Object::Base* CreateObject(std::wstring modelPath);
		void ObjectAwake(Engine::Object::Base* ob1);
		void CameraMove();
		float cameraspeed = 0.05f;

		SkyBox* skybox;

		double deltaX = 0.0f;
		double deltaY = 0.0f;
		bool flag;

		std::vector< Engine::Object::Base* > Grids;
		std::vector< Engine::Object::Base* > renderList;
		//std::vector< Render::Mesh* > AlphaRenderList;
		std::vector< Engine::Object::Base* > objectList;

	};

}