#include "pch.h"
#include "Input.h"


Engine::Input* Engine::Input::m_instance = nullptr;

Engine::Input* Engine::Input::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new Engine::Input();
    }
    return m_instance;
}

void Engine::Input::ReleaseInstance()
{
    if (m_instance != nullptr)
    {
        delete m_instance;
        m_instance = nullptr;
    }
}

Engine::Input::~Input()
{
    // ����̽� ����
    if (m_pKeyboard)
    {
        m_pKeyboard->Unacquire();
        m_pKeyboard->Release();
    }

    if (m_pMouse)
    {
        m_pMouse->Unacquire();
        m_pMouse->Release();
    }

    // DirectEngine::Input ��ü ����
    if (m_pDirectInput)
        m_pDirectInput->Release();
}

bool Engine::Input::Initialize(HINSTANCE _hInstance,HWND _hWnd)
{
    hInstance = _hInstance;
    hWnd = _hWnd;

	HRESULT hr = S_OK;
	// DirectEngine::Input ��ü ����
	DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDirectInput, nullptr);
	// Ű���� ����̽� ����
	if (SUCCEEDED(hr))
	{
		m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, nullptr); 
		m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
		m_pKeyboard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	}
	// ���콺 ����̽� ����
	if (SUCCEEDED(hr))
	{
		m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr);
		m_pMouse->SetDataFormat(&c_dfDIMouse);
		m_pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	}
 /*   if (SUCCEEDED(hr))
	{
		m_pDirectEngine::Input->CreateDevice(EnumDisplayDevices, &m_pJoystick, nullptr);
		m_pMouse->SetDataFormat(&c_dfDIMouse);
		m_pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	}*/
    
    // Ű����� ���콺 ����̽� ȹ��
    if (SUCCEEDED(hr))
    {
    m_pKeyboard->Acquire();

    m_pMouse->Acquire();

    }
    if (SUCCEEDED(hr))
    {
        return true;
    }
    else if (FAILED(hr))
    {
        return false;

    }

}

void Engine::Input::Release()
{
    // ����̽� ����
    m_pKeyboard->Unacquire();
    m_pMouse->Unacquire();
}

bool Engine::Input::ReadKeyboard()
{
    // Ű���� ���� �б�
    for (size_t i = 0; i < 256; i++)
    {
        m_prevkeyboardState[i] = m_currkeyboardState[i];
    }


    HRESULT hr = m_pKeyboard->GetDeviceState(sizeof(m_currkeyboardState), (LPVOID)&m_currkeyboardState);
    if (FAILED(hr))
    {
        // ����̽� ��ȹ��
        if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
        {
            m_pKeyboard->Acquire();
        }
        return false;
    }

    return true;
}

bool Engine::Input::ReadMouse()
{
    // ���콺 ���� �б�
    m_prevmouseState = m_currmouseState;
    HRESULT hr = m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE), &m_currmouseState);
    if (FAILED(hr))
    {
        // ����̽� ��ȹ��
        if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
        {
            m_pMouse->Acquire();
        }
        return false;
    }

    return true;
}

bool Engine::Input::IsKeyPressed(int virtualKey) const
{
    // Ű���� ���� Ȯ��
    return (m_prevkeyboardState[virtualKey] & 0x80) == 0 &&
        (m_currkeyboardState[virtualKey] & 0x80) != 0;
}
bool Engine::Input::IsKeyReleased(int virtualKey) const
{
    return (m_prevkeyboardState[virtualKey] & 0x80) != 0 &&
        (m_currkeyboardState[virtualKey] & 0x80) == 0;
}
bool Engine::Input::IsKeyHold(int virtualKey) const
{
    // Ű���� ���� Ȯ��
    return (m_prevkeyboardState[virtualKey] & 0x80) != 0 &&
        (m_currkeyboardState[virtualKey] & 0x80) != 0;
}




const DIMOUSESTATE& Engine::Input::GetMouseState() const
{
    return m_currmouseState;
}
bool Engine::Input::IsMouseButtonClicked(int mouseButton) const
{
    // Ű���� ���� Ȯ��
    return (m_currmouseState.rgbButtons[mouseButton] & 0x80) != 0;
}

LONG Engine::Input::GetMouseScroll() const
{
    return m_currmouseState.lZ;
}
