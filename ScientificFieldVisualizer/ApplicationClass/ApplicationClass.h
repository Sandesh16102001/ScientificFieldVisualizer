#pragma once

#include"../D3DClass/D3DClass.h"
#include"../CameraClass/CameraClass.h"
#include"../ModelClass/ModelClass.h"
#include"../ShaderClass/ShaderClass.h"
#include"../InputClass/InputClass.h"
#include"../ComputeShaderClass/ComputeShaderClass.h"
#include"../NormalComputeClass/NormalComputeClass.h"

const bool  FULL_SCREEN    = false;
const bool  VSYNC_ENABLED  = true;
const float SCREEN_DEPTH   = 1000.0f;
const float SCREEN_NEAR    = 0.3f;

// ---- Per-frame input speed constants (tune here, not buried in Render) ----
static constexpr float SCALAR_NUDGE_SPEED      = 0.003f;  // how fast scalar min/max shifts per frame
static constexpr float DISPLACEMENT_NUDGE_SPEED = 0.005f; // how fast deformation amplitude changes
static constexpr float ROTATION_SPEED          = 0.02f;   // radians per frame for arrow-key rotation
static constexpr float ISOLINE_NUDGE_SPEED     = 0.002f;  // isoline interval change per frame
static constexpr float ISOLINE_INTERVAL_MIN    = 0.01f;   // densest allowed contour spacing
static constexpr float ISOLINE_INTERVAL_MAX    = 0.5f;    // sparsest allowed contour spacing
static constexpr float SCALAR_RANGE_MIN_GAP    = 0.05f;   // minimum gap between scalarMin and scalarMax



class ApplicationClass
{

public : 
	ApplicationClass();
	ApplicationClass(const ApplicationClass& applicationClass);
	~ApplicationClass();


	bool Initialize(int , int , HWND);
	void ShutDown();
	bool Frame(InputClass* input);

private:
	bool Render(InputClass* input);

	// Render() helpers — each has a single responsibility
	void HandleInput(InputClass* input);          // all key-toggle and nudge logic
	void UpdateWorldMatrix(XMMATRIX& worldMatrix); // applies m_rotationX/Y to worldMatrix


private:

	D3DClass*			 m_Direct3D;
	CameraClass*		 m_Camera;
	ModelClass*			 m_Model;
	ShaderClass*		 m_Shader;
	ComputeShaderClass*  m_ComputeShader;  // per-face scalar smoothing pass
	NormalComputeClass*  m_NormalCompute;  // per-face normal recompute pass

	int					 m_colormapIndex;
	bool				 m_prevCPressed;
	int					 m_wireframeOn;
	bool				 m_prevWPressed;
	int					 m_isolineOn;       // 0=off, 1=on
	bool				 m_prevIPressed;
	float				 m_scalarMin;
	float				 m_scalarMax;
	float				 m_isolineInterval;  // contour spacing, [ ] keys
	float				 m_displacementAmplitude;
	float				 m_rotationY;        // object rotation around Y axis, arrow left/right
	float				 m_rotationX;        // object rotation around X axis, arrow up/down




};
