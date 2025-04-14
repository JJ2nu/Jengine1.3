#include "pch.h"
#include "Engine.h"


Engine::EngineCore* Engine::EngineCore::m_pInstance = nullptr;
HWND Engine::EngineCore::m_hWnd;
HWND Engine::EngineCore::imguiWnd;

LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{



	return  Engine::EngineCore::m_pInstance->WndProc(hWnd, message, wParam, lParam);
}

Engine::EngineCore::EngineCore(HINSTANCE hInstance)
	:m_hInstance(hInstance), m_szWindowClass(L"DefaultWindowClass"), m_szTitle(L"Engine::EngineCore"), m_ClientWidth(1024), m_ClientHeight(768)
{
	Engine::EngineCore::m_pInstance = this;
	m_wcex.hInstance = hInstance;
	m_wcex.cbSize = sizeof(WNDCLASSEX);
	m_wcex.style = CS_HREDRAW | CS_VREDRAW;
	m_wcex.lpfnWndProc = DefaultWndProc;
	m_wcex.cbClsExtra = 0;
	m_wcex.cbWndExtra = 0;
	m_wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	m_wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	m_wcex.lpszClassName = m_szWindowClass;

}

Engine::EngineCore::~EngineCore()
{

}


bool Engine::EngineCore::Initialize(UINT Width, UINT Height)
{
	m_ClientWidth = Width;
	m_ClientHeight = Height;

	// 등록
	RegisterClassExW(&m_wcex);

	// 원하는 크기가 조정되어 리턴
	RECT rcClient = { 0, 0, (LONG)Width, (LONG)Height };
	AdjustWindowRect(&rcClient, WS_OVERLAPPEDWINDOW, FALSE);


	m_hWnd = CreateHWND(WS_OVERLAPPEDWINDOW, Width, Height, L"MainWindow" , 100,100);

	if (!m_hWnd)
		return false;

	Render::window mainWindow;
	mainWindow = { &m_hWnd, static_cast<float>(Width), static_cast<float>(Height) };
	windowList.push_back(mainWindow);

	imguiWnd = CreateHWND(WS_POPUP | WS_VISIBLE, 800, Height+200,L"imguiWindow", 100+Width,100);
	if (!imguiWnd)
		return false;

	Render::window imguiwindow;
	imguiwindow = { &imguiWnd, static_cast<float>(800), static_cast<float>(Height) };
	windowList.push_back(imguiwindow);

	m_currentTime = m_previousTime = static_cast<float>(GetTickCount64()) / 1000.0f;
	m_Renderer = new Render::D3DRenderer(windowList);
	m_Renderer->Initialize();

	m_Loader = new Render::FBXLoader();
	m_Loader->Initialize(m_Renderer);

	m_input= Input::GetInstance();
	m_input->Initialize(m_hInstance, *mainWindow.m_hwnd);

	return true;
}

bool Engine::EngineCore::Run()
{
	// PeekMessage 메세지가 있으면 true,없으면 false
	while (TRUE)
	{
		if (PeekMessage(&m_msg, NULL, 0, 0, PM_REMOVE))
		{
			if (m_msg.message == WM_QUIT)
				break;
	
			//윈도우 메시지 처리 
			TranslateMessage(&m_msg); // 키입력관련 메시지 변환  WM_KEYDOWN -> WM_CHAR
			DispatchMessage(&m_msg);
		}
		else
		{
			m_Timer.Tick();
			Update(m_Timer.DeltaTime());

			for(size_t i = 0;i<windowList.size();i++)
			{
				Render(i);
			}
		}
	}
	return 0;
}



void Engine::EngineCore::Update(float deltaTime)
{
	m_input->ReadKeyboard();
	m_input->ReadMouse();

	if (windowList.size() > 1)
	{
		
	}




	m_Renderer->Update(deltaTime);
}

void Engine::EngineCore::Render(int windowIdx)
{
}

LRESULT CALLBACK Engine::EngineCore::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

HWND Engine::EngineCore::CreateHWND(UINT style,int width,int height, LPCWSTR title , int x, int y)
{

	m_wcex.cbSize = sizeof(WNDCLASSEX);
	m_wcex.style = style;
	m_wcex.lpfnWndProc = DefaultWndProc;
	m_wcex.cbClsExtra = 0;
	m_wcex.cbWndExtra = 0;
	m_wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	m_wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	m_wcex.lpszClassName = m_szWindowClass;

	RegisterClassEx(&m_wcex);

	HWND result = CreateWindowW(m_szWindowClass, title, style, x, y, width, height,	nullptr, nullptr, m_hInstance, nullptr);
	if (!result) return nullptr;
	ShowWindow(result, SW_SHOW);
	UpdateWindow(result);

	return result;
}