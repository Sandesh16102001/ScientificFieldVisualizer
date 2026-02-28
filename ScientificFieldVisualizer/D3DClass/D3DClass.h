#pragma once


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


#include<d3d11.h>
#include<DirectXMath.h>
using namespace DirectX;


class D3DClass
{
public : 
	D3DClass();
	D3DClass(const D3DClass& d3dclass);
	~D3DClass();

	bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullsceen, float screendepth, float screenNear);
	void ShutDown();

	void BeginScene(float red, float green, float blue, float alpha);
	void EndScene();


	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(XMMATRIX& projectionMatrix);
	void GetWorldMatrix(XMMATRIX& worldMatrix);
	void GetOrthoMatrix(XMMATRIX& orthoMatrix );

	void GetVideoCardInfo(char* name , int& memory);
	
	void SetBackBuffer();
	void SetViewPort();


private: 

	bool							m_VsyncEnabled;
	int								m_VideoCardMemory;
	char							m_VideoCardDescription[128];
	
	IDXGISwapChain*					m_SwapChain;
	ID3D11Device*					m_Device;
	ID3D11DeviceContext*			m_DeviceContext;

	ID3D11RenderTargetView*			m_RTV;
	ID3D11Texture2D*				m_DepthStencilBuffer;
	ID3D11DepthStencilState*		m_DepthStencilState;
	ID3D11DepthStencilView*			m_DepthStencilView;

	ID3D11RasterizerState*			m_RasterizerState;

	XMMATRIX						m_ProjectionMatrix;
	XMMATRIX						m_WorldMatrix;
	XMMATRIX						m_OrthoMatrix;

	D3D11_VIEWPORT					m_ViewPort;




};
