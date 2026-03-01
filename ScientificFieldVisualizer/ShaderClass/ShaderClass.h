#pragma once

#include<d3d11.h>
#include<d3dcompiler.h>
#include<DirectXMath.h>
#include<fstream>

using namespace DirectX;





class ShaderClass
{
private  :

	struct MatrixBufferTye
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		float displacementAmplitude;
		float pad[3];
	};

	struct ShaderParamsType
	{
		int      colormapIndex;    // 0 = Viridis, 1 = Jet
		int      wireframeOn;      // 0 = off,     1 = on
		int      isolineOn;        // 0 = off,     1 = on
		int      pad;              // pad to 16-byte boundary
		float    scalarMin;        // visible range lower bound
		float    scalarMax;        // visible range upper bound
		float    isolineInterval;  // spacing between contour lines
		float    pad2;             // pad to 32 bytes
		XMFLOAT3 cameraPos;       // world-space eye for Blinn-Phong specular
		float    pad3;             // pad to 48 bytes
	};

public:
	// All per-draw parameters bundled so callers don't need a 16-argument call.
	struct DrawParams
	{
		XMMATRIX  world;
		XMMATRIX  view;
		XMMATRIX  proj;
		float     displacementAmplitude;
		int       colormapIndex;   // 0 = Viridis, 1 = Jet
		int       wireframeOn;     // 0/1
		int       isolineOn;       // 0/1
		float     scalarMin;
		float     scalarMax;
		float     isolineInterval;
		XMFLOAT3  cameraPos;      // world-space eye for Blinn-Phong
		ID3D11ShaderResourceView* scalarSRV;  // smoothed scalars   (t0)
		ID3D11ShaderResourceView* normalSRV;  // recomputed normals (t1)
	};

	ShaderClass();
	~ShaderClass();

	bool Initialize(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* context, int indexCount, const DrawParams& params);


private:

	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errormsg, HWND hwnd, WCHAR* shaderfilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, const DrawParams& params);
	void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);


private : 

	ID3D11VertexShader*		m_VertexShader;
	ID3D11PixelShader*		m_PixelShader;
	ID3D11InputLayout*		m_InputLayout;
	ID3D11Buffer*			m_MatrixBuffer;
	ID3D11Buffer*			m_ShaderParamsBuffer;
};



