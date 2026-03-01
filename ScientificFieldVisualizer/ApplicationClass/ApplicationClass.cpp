#include "ApplicationClass.h"

ApplicationClass::ApplicationClass()
{
	m_Direct3D		  = nullptr;
	m_Camera		  = nullptr;
	m_Model			  = nullptr;
	m_Shader		  = nullptr;
	m_ComputeShader   = nullptr;
	m_NormalCompute   = nullptr;
	m_colormapIndex	  = 0;
	m_prevCPressed	  = false;
	m_wireframeOn	  = 0;
	m_prevWPressed	  = false;
	m_isolineOn		  = 0;
	m_prevIPressed	  = false;
	m_scalarMin		  = 0.0f;
	m_scalarMax		  = 1.0f;
	m_isolineInterval = 0.05f;   // default: one contour line every 0.05 scalar units
	m_displacementAmplitude = 0.0f;
	m_rotationY = 0.0f;
	m_rotationX = 0.0f;
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

	// Create and initialize the compute shader (per-face scalar smoothing).
	// It borrows the scalar SRV from ModelClass and writes smoothed values
	// into its own output buffer, which the vertex shader reads each frame.
	m_ComputeShader = new ComputeShaderClass;

	result = m_ComputeShader->Initialize(
		m_Direct3D->GetDevice(), hwnd,
		m_Model->GetScalarSRV(),
		m_Model->GetIndexCount());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the compute shader.", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the normal-recompute compute shader.
	// It reads {position, displacement} per vertex from ModelClass and writes
	// post-deformation face normals that the vertex shader reads via t1.
	m_NormalCompute = new NormalComputeClass;

	result = m_NormalCompute->Initialize(
		m_Direct3D->GetDevice(), hwnd,
		m_Model->GetPosDispSRV(),
		m_Model->GetIndexCount());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the normal compute shader.", L"Error", MB_OK);
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

	if (m_ComputeShader)
	{
		m_ComputeShader->Shutdown();
		delete m_ComputeShader;
		m_ComputeShader = nullptr;
	}

	if (m_NormalCompute)
	{
		m_NormalCompute->Shutdown();
		delete m_NormalCompute;
		m_NormalCompute = nullptr;
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

	m_Camera->Update(input);
	HandleInput(input);

	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_Camera->Render();

	m_Direct3D->GetWorldMatrix(worldMatrix);
	UpdateWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	m_Model->Render(m_Direct3D->GetDeviceContext());

	// Compute pass 1: per-face scalar smoothing — VS reads result via t0
	m_ComputeShader->Dispatch(m_Direct3D->GetDeviceContext());

	// Compute pass 2: post-deformation face normals — VS reads result via t1
	m_NormalCompute->Dispatch(m_Direct3D->GetDeviceContext(), m_displacementAmplitude);

	// Build draw params struct and submit
	ShaderClass::DrawParams dp;
	dp.world                 = worldMatrix;
	dp.view                  = viewMatrix;
	dp.proj                  = projectionMatrix;
	dp.displacementAmplitude = m_displacementAmplitude;
	dp.colormapIndex         = m_colormapIndex;
	dp.wireframeOn           = m_wireframeOn;
	dp.isolineOn             = m_isolineOn;
	dp.scalarMin             = m_scalarMin;
	dp.scalarMax             = m_scalarMax;
	dp.isolineInterval       = m_isolineInterval;
	dp.cameraPos             = m_Camera->GetPosition();
	dp.scalarSRV             = m_ComputeShader->GetOutputSRV();
	dp.normalSRV             = m_NormalCompute->GetOutputSRV();

	if (!m_Shader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), dp))
		return false;

	m_Direct3D->EndScene();
	return true;
}

void ApplicationClass::HandleInput(InputClass* input)
{
	// --- Toggle keys: fire once per press, not every frame ---
	bool cDown = input->IsKeyDown('C');
	if (cDown && !m_prevCPressed) m_colormapIndex = 1 - m_colormapIndex;
	m_prevCPressed = cDown;

	bool wDown = input->IsKeyDown('W');
	if (wDown && !m_prevWPressed) m_wireframeOn = 1 - m_wireframeOn;
	m_prevWPressed = wDown;

	bool iDown = input->IsKeyDown('I');
	if (iDown && !m_prevIPressed) m_isolineOn = 1 - m_isolineOn;
	m_prevIPressed = iDown;

	// --- Continuous nudge keys ---
	if (input->IsKeyDown('1')) m_scalarMin -= SCALAR_NUDGE_SPEED;
	if (input->IsKeyDown('2')) m_scalarMin += SCALAR_NUDGE_SPEED;
	if (input->IsKeyDown('3')) m_scalarMax -= SCALAR_NUDGE_SPEED;
	if (input->IsKeyDown('4')) m_scalarMax += SCALAR_NUDGE_SPEED;
	// Enforce [0..1] range and minimum gap between min and max
	m_scalarMin = max(0.0f,  min(m_scalarMin, m_scalarMax - SCALAR_RANGE_MIN_GAP));
	m_scalarMax = min(1.0f,  max(m_scalarMax, m_scalarMin + SCALAR_RANGE_MIN_GAP));

	if (input->IsKeyDown('U')) m_displacementAmplitude += DISPLACEMENT_NUDGE_SPEED;
	if (input->IsKeyDown('J')) m_displacementAmplitude -= DISPLACEMENT_NUDGE_SPEED;

	// Arrow keys — object rotation
	if (input->IsKeyDown(VK_LEFT))  m_rotationY -= ROTATION_SPEED;  // ←
	if (input->IsKeyDown(VK_RIGHT)) m_rotationY += ROTATION_SPEED;  // →
	if (input->IsKeyDown(VK_UP))    m_rotationX -= ROTATION_SPEED;  // ↑
	if (input->IsKeyDown(VK_DOWN))  m_rotationX += ROTATION_SPEED;  // ↓

	// [ / ] — isoline interval
	if (input->IsKeyDown(VK_OEM_4)) m_isolineInterval -= ISOLINE_NUDGE_SPEED;  // [
	if (input->IsKeyDown(VK_OEM_6)) m_isolineInterval += ISOLINE_NUDGE_SPEED;  // ]
	m_isolineInterval = max(ISOLINE_INTERVAL_MIN, min(m_isolineInterval, ISOLINE_INTERVAL_MAX));
}

void ApplicationClass::UpdateWorldMatrix(XMMATRIX& worldMatrix)
{
	// Apply object rotation on top of the base world matrix from D3DClass
	worldMatrix = XMMatrixRotationX(m_rotationX) * XMMatrixRotationY(m_rotationY);
}
