#include "CameraClass.h"

static const float PI = 3.14159265358979323846f;

CameraClass::CameraClass()
{
	m_radius  = 3.0f;
	m_theta   = PI * 0.5f;   // start level with the equator
	m_phi     = 0.0f;

	m_targetX = 0.0f;
	m_targetY = 0.0f;
	m_targetZ = 0.0f;
}

CameraClass::~CameraClass()
{
}

void CameraClass::Update(InputClass* input)
{
	const float orbitSensitivity = 0.005f;   // radians per pixel
	const float panSensitivity   = 0.003f;   // world-units per pixel (scales with radius)
	const float zoomSensitivity  = 0.001f;   // radius change per wheel unit

	int dx    = input->GetMouseDeltaX();
	int dy    = input->GetMouseDeltaY();
	int wheel = input->GetMouseWheelDelta();

	// --- Left drag: orbit ---
	if (input->IsMouseLeftDown())
	{
		m_phi   += (float)dx * orbitSensitivity;
		m_theta -= (float)dy * orbitSensitivity;

		// Clamp theta so camera never flips at the poles
		if (m_theta < 0.01f)      m_theta = 0.01f;
		if (m_theta > PI - 0.01f) m_theta = PI - 0.01f;
	}

	// --- Right drag: pan (slide the orbit target) ---
	if (input->IsMouseRightDown() && (dx != 0 || dy != 0))
	{
		// Camera-right and camera-up vectors derived from spherical angles
		float sinPhi   = sinf(m_phi),   cosPhi   = cosf(m_phi);
		float sinTheta = sinf(m_theta), cosTheta = cosf(m_theta);

		// Right = tangent in phi direction: d(pos)/d(phi) normalised
		float rX = -sinPhi;
		float rY =  0.0f;
		float rZ =  cosPhi;

		// Up = tangent in -theta direction: d(pos)/d(-theta) normalised
		float uX = -cosTheta * cosPhi;
		float uY =  sinTheta;
		float uZ = -cosTheta * sinPhi;

		float panScale = m_radius * panSensitivity;

		m_targetX += (-dx * rX + dy * uX) * panScale;
		m_targetY += (-dx * rY + dy * uY) * panScale;
		m_targetZ += (-dx * rZ + dy * uZ) * panScale;
	}

	// --- Scroll wheel: zoom ---
	if (wheel != 0)
	{
		m_radius -= (float)wheel * zoomSensitivity;
		if (m_radius < 0.5f)  m_radius = 0.5f;
		if (m_radius > 50.0f) m_radius = 50.0f;
	}
}

void CameraClass::Render()
{
	float sinTheta = sinf(m_theta), cosTheta = cosf(m_theta);
	float sinPhi   = sinf(m_phi),   cosPhi   = cosf(m_phi);

	// Cartesian camera position from spherical coords + target offset
	float camX = m_targetX + m_radius * sinTheta * cosPhi;
	float camY = m_targetY + m_radius * cosTheta;
	float camZ = m_targetZ + m_radius * sinTheta * sinPhi;

	XMVECTOR eye    = XMVectorSet(camX, camY, camZ, 1.0f);
	XMVECTOR target = XMVectorSet(m_targetX, m_targetY, m_targetZ, 1.0f);
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// Near a pole the Y-up becomes degenerate; switch to Z-up
	if (sinTheta < 0.001f)
		up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	m_ViewMatrix = XMMatrixLookAtLH(eye, target, up);
}

void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_ViewMatrix;
}
