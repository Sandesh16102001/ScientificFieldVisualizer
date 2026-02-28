#include "D3DClass.h"

D3DClass::D3DClass()
{
	m_SwapChain					= nullptr;
	m_Device					= nullptr;
	m_DeviceContext				= nullptr;

	m_RTV						= nullptr;
	m_DepthStencilBuffer		= nullptr;
	m_DepthStencilState			= nullptr;
	m_DepthStencilView			= nullptr;

	m_RasterizerState			= nullptr;
}

D3DClass::D3DClass(const D3DClass& d3dclass)
{
}

D3DClass::~D3DClass()
{
}

bool D3DClass::Initialize(int screenWidth , int screenHeight , bool vsync , HWND hwnd , bool fullsceen , float screendepth  , float screenNear )
{
	HRESULT hr;

	IDXGIFactory*	factory;
	IDXGIAdapter*	adapter;
	IDXGIOutput*	adapterOutput;


	unsigned int numModes, i, numerator, denominator;
	unsigned long long stringLength;

	DXGI_MODE_DESC* diplayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBuffer;

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterizerDesc;
	float fieldOfView, screenAspect;

	m_VsyncEnabled = vsync;


	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(hr))
		return false;

	hr = factory->EnumAdapters(0, &adapter);
	if (FAILED(hr))
		return false;

	hr = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(hr))
		return false;

	hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
	if (FAILED(hr))
		return false;

	diplayModeList = new DXGI_MODE_DESC[numModes];


	hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, diplayModeList);
	if (FAILED(hr))
		return false;


	for (i = 0; i < numModes; i++)
	{
		if (diplayModeList[i].Width == screenWidth)
		{
			if (diplayModeList[i].Height == screenHeight)
			{
				numerator	= diplayModeList[i].RefreshRate.Numerator;
				denominator = diplayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	//Get the adapter/VideoCard description


	hr = adapter->GetDesc(&adapterDesc);
	if (FAILED(hr))
		return false;

	//Store the dedicated video card memory in megabites 
	m_VideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	//Convert the name of the video card and to a character array and store it 
	
	error = wcstombs_s(&stringLength, m_VideoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
		return false;


	delete[] diplayModeList;
	diplayModeList = nullptr;

	adapterOutput->Release();
	adapterOutput = nullptr;

	adapter->Release();
	adapter = nullptr;

	factory->Release();
	factory = nullptr;


	//Initializing the swap Chain Desc 
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));


	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// Set the refresh rate of the back buffer.
	if (m_VsyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;

	// multisampling off
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.Windowed = fullsceen ? false : true;

	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	featureLevel = D3D_FEATURE_LEVEL_11_0;

	hr = D3D11CreateDeviceAndSwapChain(adapter, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_SwapChain, &m_Device, NULL, &m_DeviceContext);


	if (FAILED(hr))
		return false;

	hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (FAILED(hr))
		return false;

	hr = m_Device->CreateRenderTargetView(backBuffer, NULL, &m_RTV);
	if (FAILED(hr))
		return false;

	backBuffer->Release();
	backBuffer = nullptr;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	hr = m_Device->CreateTexture2D(&depthBufferDesc, NULL, &m_DepthStencilBuffer);
	if (FAILED(hr))
		return false;

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;


	hr = m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
	if (FAILED(hr))
		return false;


	m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState, 1);

	// Initialize the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	hr = m_Device->CreateDepthStencilView(m_DepthStencilBuffer, &depthStencilViewDesc, &m_DepthStencilView);

	if (FAILED(hr))
		return false;

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_DeviceContext->OMSetRenderTargets(1, &m_RTV, m_DepthStencilView);

	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	hr = m_Device->CreateRasterizerState(&rasterizerDesc, &m_RasterizerState);
	if (FAILED(hr))
	{
		return false;
	}

	// Now set the rasterizer state.
	m_DeviceContext->RSSetState(m_RasterizerState);

	// Setup the viewport for rendering.
	m_ViewPort.Width = (float)screenWidth;
	m_ViewPort.Height = (float)screenHeight;
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;
	m_ViewPort.TopLeftX = 0.0f;
	m_ViewPort.TopLeftY = 0.0f;

	// Create the viewport.
	m_DeviceContext->RSSetViewports(1, &m_ViewPort);


	// Setup the projection matrix.
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// Create the projection matrix for 3D rendering.
	m_ProjectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screendepth);

	m_WorldMatrix = XMMatrixIdentity();

	// Create an orthographic projection matrix for 2D rendering.
	m_OrthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screendepth);

	return true;
}

void D3DClass::ShutDown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_SwapChain)
	{
		m_SwapChain->SetFullscreenState(false, NULL);
	}

	if (m_RasterizerState)
	{
		m_RasterizerState->Release();
		m_RasterizerState = nullptr;
	}

	if (m_DepthStencilView)
	{
		m_DepthStencilView->Release();
		m_DepthStencilView = nullptr;
	}

	if (m_DepthStencilState)
	{
		m_DepthStencilState->Release();
		m_DepthStencilState = nullptr;
	}

	if (m_DepthStencilBuffer)
	{
		m_DepthStencilBuffer->Release();
		m_DepthStencilBuffer = nullptr;
	}

	if (m_RTV)
	{
		m_RTV->Release();
		m_RTV = nullptr;
	}

	if (m_DeviceContext)
	{
		m_DeviceContext->Release();
		m_DeviceContext = nullptr;
	}

	if (m_Device)
	{
		m_Device->Release();
		m_Device = nullptr;
	}

	if (m_SwapChain)
	{
		m_SwapChain->Release();
		m_SwapChain = nullptr;
	}

	return;
}

void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];

	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	//Clearing back buffer 
	m_DeviceContext->ClearRenderTargetView(m_RTV, color);

	//Claring depth buffer
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;


}

void D3DClass::EndScene()
{
	if (m_VsyncEnabled)
	{
		m_SwapChain->Present(1, 0);
	}
	else
	{
		m_SwapChain->Present(0, 0);
	}
}

ID3D11Device* D3DClass::GetDevice()
{
	return m_Device;
}

ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_DeviceContext;
}

void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_ProjectionMatrix;
}

void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_WorldMatrix;
}

void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_OrthoMatrix;
}

void D3DClass::GetVideoCardInfo(char* name, int& memory)
{
	strcpy_s(name, 128, m_VideoCardDescription);
	memory = m_VideoCardMemory;
}

void D3DClass::SetBackBuffer()
{
	m_DeviceContext->OMSetRenderTargets(1, &m_RTV, m_DepthStencilView);
}

void D3DClass::SetViewPort()
{
	m_DeviceContext->RSSetViewports(1, &m_ViewPort);
}






