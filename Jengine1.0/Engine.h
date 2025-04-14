#pragma once

#include "../D3D11Render/D3D11Render.h"
#include "../D3D11Render/FBXLoader.h"
#define MAX_LOADSTRING 100

namespace Engine
{
	class EngineCore
	{
	public:
		EngineCore(HINSTANCE hInstance);
		virtual ~EngineCore();

		static HWND m_hWnd;		//�����ʿ��ϴ� ������ ���������� ���ϱ����� ��������� �������.
		static HWND imguiWnd;		
		static EngineCore* m_pInstance;			// �����ڿ��� �ν��Ͻ� �����͸� �����Ѵ�.

	public:
		HACCEL m_hAccelTable;
		MSG m_msg;
		HINSTANCE m_hInstance;                       
		WCHAR m_szTitle[MAX_LOADSTRING];             
		WCHAR m_szWindowClass[MAX_LOADSTRING];       
		WNDCLASSEXW m_wcex;
		float m_previousTime;
		float m_currentTime;
		int  m_nCmdShow;
		Time m_Timer;
		UINT m_ClientWidth;
		UINT m_ClientHeight;

		std::vector<Render::window> windowList;
		Render::D3DRenderer* m_Renderer;
		Render::FBXLoader* m_Loader;
		Engine::Input* m_input;


	public:
		HWND CreateHWND(UINT style, int Width, int height,  LPCWSTR title, int x, int y);


		virtual bool Initialize(UINT Width, UINT Height);
		virtual bool Run();
		virtual void Update(float deltatime); 
		virtual void Render(int windowIdx) = 0; 

		virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
}

