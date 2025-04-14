#pragma once

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")
#include <dinput.h>
namespace Engine
{

	class Input
	{
	public:
		static Input* m_instance;

		HINSTANCE hInstance;
		HWND hWnd;
		static Input* GetInstance();
		void ReleaseInstance();

		bool Initialize(HINSTANCE hInstance, HWND hWnd);
		void Release();

		bool ReadKeyboard();
		bool ReadMouse();

		bool IsKeyPressed(int virtualKey) const;
		bool IsKeyReleased(int virtualKey) const;
		bool IsKeyHold(int virtualKey) const;
		const DIMOUSESTATE& GetMouseState() const;

		bool IsMouseButtonClicked(int mouseButton) const;
		LONG GetMouseScroll() const;

		enum mouseButton
		{
			LeftButton,
			RightButton,
			ScrollButton
		};




		BYTE m_currkeyboardState[256];
		BYTE m_prevkeyboardState[256];
		DIMOUSESTATE m_prevmouseState;
		DIMOUSESTATE m_currmouseState;

		Input() {};
		~Input();
	private:

		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;


		LPDIRECTINPUT8 m_pDirectInput;
		LPDIRECTINPUTDEVICE8 m_pKeyboard;
		LPDIRECTINPUTDEVICE8 m_pMouse;
		LPDIRECTINPUTDEVICE8 m_pJoystick;


	};

}
