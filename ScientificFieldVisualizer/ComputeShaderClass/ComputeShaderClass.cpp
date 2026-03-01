#include "ComputeShaderClass.h"

ComputeShaderClass::ComputeShaderClass()
	: m_ComputeShader(nullptr)
	, m_OutputBuffer(nullptr)
	, m_OutputUAV(nullptr)
	, m_OutputSRV(nullptr)
	, m_ParamsBuffer(nullptr)
	, m_InputSRV(nullptr)
	, m_VertexCount(0)
	, m_GroupCount(0)
{
}

ComputeShaderClass::~ComputeShaderClass()
{
}

bool ComputeShaderClass::Initialize(ID3D11Device* device, HWND hwnd,
                                    ID3D11ShaderResourceView* inputSRV, int vertexCount)
{
	m_InputSRV    = inputSRV;           // borrow — ModelClass keeps ownership
	m_VertexCount = vertexCount;
	m_GroupCount  = ((UINT)vertexCount + 63) / 64;  // ceil(N/64) thread groups

	// --- Load and compile the compute shader ---
	if (!InitializeShader(device, hwnd))
		return false;

	// --- Output buffer -------------------------------------------------------
	// A structured buffer of `vertexCount` floats.
	// Bound as UAV for the CS write, then as SRV for the VS read.
	D3D11_BUFFER_DESC bd        = {};
	bd.ByteWidth                = sizeof(float) * (UINT)vertexCount;
	bd.Usage                    = D3D11_USAGE_DEFAULT;
	bd.BindFlags                = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bd.MiscFlags                = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride      = sizeof(float);

	if (FAILED(device->CreateBuffer(&bd, nullptr, &m_OutputBuffer)))
		return false;

	// UAV: compute shader writes smoothed scalars here
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format                    = DXGI_FORMAT_UNKNOWN; // structured → UNKNOWN
	uavDesc.ViewDimension             = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements        = (UINT)vertexCount;

	if (FAILED(device->CreateUnorderedAccessView(m_OutputBuffer, &uavDesc, &m_OutputUAV)))
		return false;

	// SRV: vertex shader reads from the same buffer as a structured buffer
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format                   = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension            = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.NumElements       = (UINT)vertexCount;

	if (FAILED(device->CreateShaderResourceView(m_OutputBuffer, &srvDesc, &m_OutputSRV)))
		return false;

	// --- Params constant buffer (immutable) ----------------------------------
	// Holds just `vertexCount` so the CS can guard threads past the buffer end.
	struct SmoothParams { UINT count; UINT pad[3]; };
	SmoothParams params = { (UINT)vertexCount, {0, 0, 0} };

	D3D11_BUFFER_DESC cbd   = {};
	cbd.ByteWidth           = sizeof(SmoothParams);
	cbd.Usage               = D3D11_USAGE_IMMUTABLE;
	cbd.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = { &params, 0, 0 };

	if (FAILED(device->CreateBuffer(&cbd, &initData, &m_ParamsBuffer)))
		return false;

	return true;
}

bool ComputeShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd)
{
	ID3D10Blob* shaderBlob = nullptr;
	ID3D10Blob* errorBlob  = nullptr;

	HRESULT hr = D3DCompileFromFile(
		L"ComputeShader.hlsl", nullptr, nullptr,
		"CSMain", "cs_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&shaderBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			std::ofstream f("shader-error.txt");
			char* msg = static_cast<char*>(errorBlob->GetBufferPointer());
			for (SIZE_T i = 0; i < errorBlob->GetBufferSize(); ++i)
				f << msg[i];
			f.close();
			errorBlob->Release();
			MessageBox(hwnd,
				L"Error compiling ComputeShader.hlsl — see shader-error.txt",
				L"Shader Error", MB_OK);
		}
		else
		{
			MessageBox(hwnd, L"ComputeShader.hlsl not found", L"Missing File", MB_OK);
		}
		return false;
	}

	hr = device->CreateComputeShader(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		nullptr,
		&m_ComputeShader);

	shaderBlob->Release();
	return SUCCEEDED(hr);
}

void ComputeShaderClass::Dispatch(ID3D11DeviceContext* context)
{
	// 1. Set the compute shader and its resources
	context->CSSetShader(m_ComputeShader, nullptr, 0);
	context->CSSetConstantBuffers(0, 1, &m_ParamsBuffer);
	context->CSSetShaderResources(0, 1, &m_InputSRV);
	context->CSSetUnorderedAccessViews(0, 1, &m_OutputUAV, nullptr);

	// 2. Dispatch: one thread per vertex, 64 threads per group
	context->Dispatch(m_GroupCount, 1, 1);

	// 3. Unbind CS resources so the output buffer can be used as a VS SRV
	//    D3D11 requires a resource to not be bound as UAV and SRV simultaneously.
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	ID3D11ShaderResourceView*  nullSRV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	context->CSSetShaderResources(0, 1, &nullSRV);
	context->CSSetShader(nullptr, nullptr, 0);
}

void ComputeShaderClass::Shutdown()
{
	if (m_ParamsBuffer)   { m_ParamsBuffer->Release();   m_ParamsBuffer   = nullptr; }
	if (m_OutputSRV)      { m_OutputSRV->Release();      m_OutputSRV      = nullptr; }
	if (m_OutputUAV)      { m_OutputUAV->Release();      m_OutputUAV      = nullptr; }
	if (m_OutputBuffer)   { m_OutputBuffer->Release();   m_OutputBuffer   = nullptr; }
	if (m_ComputeShader)  { m_ComputeShader->Release();  m_ComputeShader  = nullptr; }
	// m_InputSRV is borrowed — ModelClass owns and releases it
}
