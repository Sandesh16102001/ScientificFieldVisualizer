#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>

// ---------------------------------------------------------------------------
// NormalComputeClass
// ---------------------------------------------------------------------------
// Owns the per-face normal recomputation compute pass.
//
// The vertex shader deforms the mesh by adding (displacement * amplitude)
// to every position.  The original sphere normals become stale after any
// non-zero deformation, causing wrong Lambertian shading.
//
// This class runs a compute pass every frame that recalculates face normals
// from the deformed geometry.  The vertex shader then reads those normals
// from register t1 instead of from the (stale) vertex buffer field.
//
// Data flow per frame:
//   ModelClass::m_PosDispSRV  (float3 pos + float3 disp per vertex, borrowed)
//     │
//     ▼  NormalComputeShader.hlsl [64 threads/group, one thread per triangle]
//   m_OutputUAV  (recomputed float3 face normals, one per expanded vertex)
//     │
//     ▼  bound as SRV (t1) → VertexShader reads recomputedNormals[SV_VertexID]
// ---------------------------------------------------------------------------

class NormalComputeClass
{
public:
	NormalComputeClass();
	~NormalComputeClass();

	// posDispSRV — structured buffer of {float3 pos, float3 disp}, owned by ModelClass.
	bool Initialize(ID3D11Device* device, HWND hwnd,
	                ID3D11ShaderResourceView* posDispSRV, int vertexCount);

	void Shutdown();

	// Recompute face normals for the given displacement amplitude.
	// Call this every frame BEFORE the draw call.
	void Dispatch(ID3D11DeviceContext* context, float displacementAmplitude);

	// SRV over the recomputed normals — bind to VS t1 each draw call.
	ID3D11ShaderResourceView* GetOutputSRV() const { return m_OutputSRV; }

private:
	bool InitializeShader(ID3D11Device* device, HWND hwnd);

	ID3D11ComputeShader*        m_ComputeShader;

	// Output buffer: float3 per vertex (StructureByteStride = 12).
	// Bound as UAV during the CS pass, then as SRV during the draw call.
	ID3D11Buffer*               m_OutputBuffer;
	ID3D11UnorderedAccessView*  m_OutputUAV;
	ID3D11ShaderResourceView*   m_OutputSRV;

	// Dynamic cbuffer — updated every frame with the current displacementAmplitude.
	ID3D11Buffer*               m_ParamsBuffer;

	// Borrowed from ModelClass — do NOT release here.
	ID3D11ShaderResourceView*   m_InputSRV;

	int  m_VertexCount;
	UINT m_GroupCount;  // ceil(triangleCount / 64)
};
