#include"SystemClass/SystemClass.h"

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE instancePrev, PSTR pScmdLine, int iCmdShow)
{

	SystemClass* System; 
	bool result;

	System = new SystemClass(); 


	result = System->Initialize();

	if (result)
	{
		System->Run();
	}

	System->ShutDown();

	delete System;
	System = nullptr;


	return 0; 

}