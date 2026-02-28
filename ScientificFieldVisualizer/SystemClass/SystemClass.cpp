#include"SystemClass.h"

SystemClass::SystemClass()
{
	m_Input				= nullptr;
	m_Application		= nullptr;
}


SystemClass::SystemClass(const SystemClass& systemClass)
{

}

SystemClass::~SystemClass()
{

}

bool SystemClass::Initialize()
{
	int ScreenWidth  = 0; 
	int ScreenHeight = 0;

	InitializeWindow(ScreenWidth, ScreenHeight);

	m_Input = new InputClass();

	m_Input->Initialize();

	m_Application = new ApplicationClass();

	bool result = m_Application->Initialize(ScreenWidth, ScreenHeight , m_Hwnd);

	if (!result)
		return false;

	return true;
} 

void SystemClass::ShutDown()
{
	if (m_Application)
	{
		delete m_Application;
		m_Application = nullptr;
	}

	if (m_Input)
	{
		delete m_Input;
		m_Input = nullptr;
	}

	ShutDownWindow();
}


void SystemClass::Run()
{
	MSG msg;
	bool done, result; 


	ZeroMemory(&msg, sizeof(MSG));

	done = false;

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			result = Frame();
			if (!result)
				done = true; 
		}
	}
	return;
}


bool SystemClass::Frame()
{
	bool result; 

	if (m_Input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	result = m_Application->Frame(m_Input);
	if(!result)
		return false ; 

	// Reset per-frame mouse deltas AFTER processing so next frame starts clean
	m_Input->BeginFrame();

	return true;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		// Check if a key has been pressed on the keyboard.
	case WM_KEYDOWN:
	{
		m_Input->KeyDown((unsigned int)wparam);
		return 0;
	}

	case WM_KEYUP:
	{
		m_Input->KeyUp((unsigned int)wparam);
		return 0;
	}

	case WM_MOUSEMOVE:
	{
		m_Input->MouseMove((int)LOWORD(lparam), (int)HIWORD(lparam));
		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		m_Input->MouseLeftDown((int)LOWORD(lparam), (int)HIWORD(lparam));
		return 0;
	}

	case WM_LBUTTONUP:
	{
		m_Input->MouseLeftUp((int)LOWORD(lparam), (int)HIWORD(lparam));
		return 0;
	}

	case WM_RBUTTONDOWN:
	{
		m_Input->MouseRightDown((int)LOWORD(lparam), (int)HIWORD(lparam));
		return 0;
	}

	case WM_RBUTTONUP:
	{
		m_Input->MouseRightUp((int)LOWORD(lparam), (int)HIWORD(lparam));
		return 0;
	}

	case WM_MOUSEWHEEL:
	{
		m_Input->MouseWheel((int)GET_WHEEL_DELTA_WPARAM(wparam));
		return 0;
	}

	// Any other messages send to the default message handler as our application won't make use of them.
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}


void SystemClass::InitializeWindow(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// Get an external pointer to this object.	
	ApplicationHandle = this;

	// Get the instance of this application.
	m_Hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_ApplicationName = L"ScientificFieldVisualizer";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_Hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_ApplicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		screenWidth = 800;
		screenHeight = 600;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_Hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_ApplicationName, m_ApplicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_Hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_Hwnd, SW_SHOW);
	SetForegroundWindow(m_Hwnd);
	SetFocus(m_Hwnd);

	// Mouse cursor remains visible for orbit/pan interaction

	return;
}


void SystemClass::ShutDownWindow()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_Hwnd);
	m_Hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_ApplicationName, m_Hinstance);
	m_Hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// All other messages pass to the message handler in the system class.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}