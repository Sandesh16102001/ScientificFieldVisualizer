#pragma once

#include"../D3DClass/D3DClass.h"
#include"../CameraClass/CameraClass.h"
#include"../ModelClass/ModelClass.h"
#include"../ShaderClass/ShaderClass.h"
#include"../InputClass/InputClass.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;



class ApplicationClass
{

public : 
	ApplicationClass();
	ApplicationClass(const ApplicationClass& applicationClass);
	~ApplicationClass();


	bool Initialize(int , int , HWND);
	void ShutDown();
	bool Frame(InputClass* input);

private : 
	bool Render(InputClass* input);


private:

	D3DClass*			 m_Direct3D;
	CameraClass*		 m_Camera;
	ModelClass*			 m_Model;
	ShaderClass*		 m_Shader;

	int					 m_colormapIndex;
	bool				 m_prevCPressed;
	int					 m_wireframeOn;
	bool				 m_prevWPressed;
	float				 m_scalarMin;      // visible range: 0.0 .. 1.0
	float				 m_scalarMax;




};
