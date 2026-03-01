#pragma once

#include <directxmath.h>
#include "../InputClass/InputClass.h"

using namespace DirectX;

class CameraClass
{
public:
	CameraClass();
	~CameraClass();

	// Call every frame before Render() to apply mouse input
	void Update(InputClass* input);

	// Builds the view matrix from current spherical state
	void Render();
	void GetViewMatrix(XMMATRIX& viewMatrix);
	XMFLOAT3 GetPosition() const { return XMFLOAT3(m_posX, m_posY, m_posZ); }

private:
	// Spherical-coordinate orbital camera
	float    m_radius;     // distance from target (zoom level)
	float    m_theta;      // polar angle:      0 = top pole, PI = bottom pole
	float    m_phi;        // azimuthal angle:  0 = +Z axis, sweeps around Y

	// World-space point the camera orbits around
	float    m_targetX;
	float    m_targetY;
	float    m_targetZ;

	// Cached cartesian eye position (valid after Render() each frame)
	float    m_posX;
	float    m_posY;
	float    m_posZ;

	XMMATRIX m_ViewMatrix;
};