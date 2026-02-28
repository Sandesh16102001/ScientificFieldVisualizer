#include "ApplicationClass.h"

ApplicationClass::ApplicationClass()
{
	m_Direct3D		= nullptr;
	m_Camera		= nullptr;
	m_Model			= nullptr;
	m_Shader		= nullptr;
	m_colormapIndex	= 0;
	m_prevCPressed	= false;
	m_wireframeOn	= 0;
	m_prevWPressed	= false;
	m_scalarMin		= 0.0f;
	m_scalarMax		= 1.0f;
}

ApplicationClass::ApplicationClass(const ApplicationClass& applicationClass)
{
}

ApplicationClass::~ApplicationClass()
{
}

bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{

	bool result;

	m_Direct3D = new D3DClass();

	result = m_Direct3D->Initialize(screenWidth , screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN , SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}



	// Create the camera object.
	m_Camera = new CameraClass;
	// Create and initialize the model object.
	m_Model = new ModelClass;

	result = m_Model->Initialize(m_Direct3D->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the color shader object.
	m_Shader = new ShaderClass;

	result = m_Shader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}



	return true; 
}

void ApplicationClass::ShutDown()
{
	if (m_Direct3D) 
	{
		m_Direct3D->ShutDown();
		delete m_Direct3D; 
		m_Direct3D = nullptr;
	}

	if (m_Shader)
	{
		m_Shader->Shutdown();
		delete m_Shader;
		m_Shader = nullptr;
	}

	// Release the model object.
	if (m_Model)
	{
		m_Model->ShutDown();
		delete m_Model;
		m_Model = nullptr;
	}

	// Release the camera object.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = nullptr;
	}
	return;
}

bool ApplicationClass::Frame(InputClass* input)
{
	bool result; 

	result = Render(input);
	if (!result)
		return false;
	return true;
}

bool ApplicationClass::Render(InputClass* input)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	// Update orbital camera from mouse input
	m_Camera->Update(input);

	// Toggle colormap: C key
	bool cDown = input->IsKeyDown('C');
	if (cDown && !m_prevCPressed)
		m_colormapIndex = 1 - m_colormapIndex;
	m_prevCPressed = cDown;

	// Toggle wireframe: W key
	bool wDown = input->IsKeyDown('W');
	if (wDown && !m_prevWPressed)
		m_wireframeOn = 1 - m_wireframeOn;
	m_prevWPressed = wDown;

	// Scalar range nudge (held keys, continuous): 1/2 = move min, 3/4 = move max
	const float nudge = 0.003f;
	if (input->IsKeyDown('1')) m_scalarMin -= nudge;
	if (input->IsKeyDown('2')) m_scalarMin += nudge;
	if (input->IsKeyDown('3')) m_scalarMax -= nudge;
	if (input->IsKeyDown('4')) m_scalarMax += nudge;
	// Clamp: keep min in [0, max-0.05] and max in [min+0.05, 1]
	if (m_scalarMin < 0.0f)              m_scalarMin = 0.0f;
	if (m_scalarMin > m_scalarMax-0.05f) m_scalarMin = m_scalarMax - 0.05f;
	if (m_scalarMax > 1.0f)              m_scalarMax = 1.0f;
	if (m_scalarMax < m_scalarMin+0.05f) m_scalarMax = m_scalarMin + 0.05f;

	m_Direct3D->BeginScene(0.0f,0.0f, 0.0f , 1 );

	m_Camera->Render();

	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	m_Model->Render(m_Direct3D->GetDeviceContext());

	result = m_Shader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_colormapIndex, m_wireframeOn, m_scalarMin, m_scalarMax);
	if (!result)
	{
		return false;
	}
	

	m_Direct3D->EndScene();

	return true;
}
