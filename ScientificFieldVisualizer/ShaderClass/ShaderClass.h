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
	};

	struct ShaderParamsType
	{
		int   colormapIndex;  // 0 = Viridis, 1 = Jet
		int   wireframeOn;    // 0 = off,     1 = on
		float scalarMin;      // visible range lower bound
		float scalarMax;      // visible range upper bound
		// Total: 16 bytes
	};

public : 
	ShaderClass();
	~ShaderClass();

	bool Initialize(ID3D11Device* device , HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* context, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, int colormapIndex, int wireframeOn, float scalarMin, float scalarMax);


private : 

	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errormsg, HWND hwnd, WCHAR* shaderfilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX world, XMMATRIX view, XMMATRIX projection, int colormapIndex, int wireframeOn, float scalarMin, float scalarMax);
	void RenderShader(ID3D11DeviceContext* deviceContext , int indexCount );


private : 

	ID3D11VertexShader*		m_VertexShader;
	ID3D11PixelShader*		m_PixelShader;
	ID3D11InputLayout*		m_InputLayout;
	ID3D11Buffer*			m_MatrixBuffer;
	ID3D11Buffer*			m_ShaderParamsBuffer;
};



