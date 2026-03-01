#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>

// ---------------------------------------------------------------------------
// ComputeShaderClass
// ---------------------------------------------------------------------------
// Owns the per-face scalar smoothing compute pass.
//
// Data flow every frame:
//   inputSRV  (original scalars, owned by ModelClass)
//     │
//     ▼  [CS: per-face average, 64 threads/group]
//   m_OutputUAV  (smoothed scalars)
//     │
//     ▼  bound as SRV → VertexShader reads smoothedScalars[SV_VertexID]
// ---------------------------------------------------------------------------

class ComputeShaderClass
{
public:
	ComputeShaderClass();
	~ComputeShaderClass();

	// inputSRV is a structured buffer of floats owned by ModelClass (borrowed ref).
	bool Initialize(ID3D11Device* device, HWND hwnd,
	                ID3D11ShaderResourceView* inputSRV, int vertexCount);

	void Shutdown();

	// Run the smoothing pass.  Must be called before the draw call each frame.
	void Dispatch(ID3D11DeviceContext* context);

	// The SRV over the smoothed output — bind this to VS register t0 each frame.
	ID3D11ShaderResourceView* GetOutputSRV() const { return m_OutputSRV; }

private:
	bool InitializeShader(ID3D11Device* device, HWND hwnd);

	ID3D11ComputeShader*        m_ComputeShader;

	// Output buffer: same size as vertex count, floats.
	// Bound as UAV during the compute pass, then as SRV during the draw call.
	ID3D11Buffer*               m_OutputBuffer;
	ID3D11UnorderedAccessView*  m_OutputUAV;
	ID3D11ShaderResourceView*   m_OutputSRV;

	// Immutable cbuffer that tells the shader how many vertices exist,
	// so threads past the end of the buffer return early.
	ID3D11Buffer*               m_ParamsBuffer;

	// Borrowed reference — owned by ModelClass.
	ID3D11ShaderResourceView*   m_InputSRV;

	int  m_VertexCount;
	UINT m_GroupCount;   // ceil(vertexCount / 64)
};
