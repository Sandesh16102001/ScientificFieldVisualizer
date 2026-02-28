#pragma once
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>

#include"../InputClass/InputClass.h"
#include"../ApplicationClass/ApplicationClass.h"

#include<string>
class SystemClass
{
public : 
	SystemClass();
	SystemClass(const SystemClass& systemClass);
	~SystemClass();

	bool Initialize();
	void ShutDown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	
private : 

	bool Frame();
	void InitializeWindow(int& width , int& height);
	void ShutDownWindow();

private : 
	LPCWSTR					m_ApplicationName;
	HINSTANCE				m_Hinstance;
	HWND					m_Hwnd;
	InputClass*				m_Input;
	ApplicationClass*		m_Application;



};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static SystemClass* ApplicationHandle = 0;

