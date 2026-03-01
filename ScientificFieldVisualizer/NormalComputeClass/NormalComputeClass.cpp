#include "NormalComputeClass.h"

NormalComputeClass::NormalComputeClass()
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

NormalComputeClass::~NormalComputeClass()
{
}

bool NormalComputeClass::Initialize(ID3D11Device* device, HWND hwnd,
                                    ID3D11ShaderResourceView* posDispSRV, int vertexCount)
{
	m_InputSRV    = posDispSRV;       // borrowed — ModelClass owns and releases it
	m_VertexCount = vertexCount;
	int triangleCount = vertexCount / 3;
	m_GroupCount = ((UINT)triangleCount + 63) / 64;  // ceil(triangles / 64)

	if (!InitializeShader(device, hwnd))
		return false;

	// --- Output buffer: float3 per expanded vertex ----------------------------
	// StructureByteStride = 12 (float3 = 3 * 4 bytes).
	// Structured buffers do NOT apply constant-buffer 16-byte padding, so
	// float3 at stride 12 is valid and matches the HLSL RWStructuredBuffer<float3>.
	D3D11_BUFFER_DESC bd        = {};
	bd.ByteWidth                = 12u * (UINT)vertexCount;  // 12 = sizeof(float3)
	bd.Usage                    = D3D11_USAGE_DEFAULT;
	bd.BindFlags                = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bd.MiscFlags                = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride      = 12;

	if (FAILED(device->CreateBuffer(&bd, nullptr, &m_OutputBuffer)))
		return false;

	// UAV — the CS writes face normals here
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format                    = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension             = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements        = (UINT)vertexCount;

	if (FAILED(device->CreateUnorderedAccessView(m_OutputBuffer, &uavDesc, &m_OutputUAV)))
		return false;

	// SRV — the VS reads from the same buffer via t1
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format                   = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension            = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.NumElements       = (UINT)vertexCount;

	if (FAILED(device->CreateShaderResourceView(m_OutputBuffer, &srvDesc, &m_OutputSRV)))
		return false;

	// --- Dynamic cbuffer (updated every frame with displacementAmplitude) ----
	// Layout: uint triangleCount + float displacementAmplitude + uint2 pad = 16 bytes
	D3D11_BUFFER_DESC cbd   = {};
	cbd.ByteWidth           = 16;
	cbd.Usage               = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(device->CreateBuffer(&cbd, nullptr, &m_ParamsBuffer)))
		return false;

	return true;
}

bool NormalComputeClass::InitializeShader(ID3D11Device* device, HWND hwnd)
{
	ID3D10Blob* shaderBlob = nullptr;
	ID3D10Blob* errorBlob  = nullptr;

	HRESULT hr = D3DCompileFromFile(
		L"NormalComputeShader.hlsl", nullptr, nullptr,
		"CSNormals", "cs_5_0",
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
				L"Error compiling NormalComputeShader.hlsl — see shader-error.txt",
				L"Shader Error", MB_OK);
		}
		else
		{
			MessageBox(hwnd, L"NormalComputeShader.hlsl not found", L"Missing File", MB_OK);
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

void NormalComputeClass::Dispatch(ID3D11DeviceContext* context, float displacementAmplitude)
{
	// 1. Update the cbuffer with this frame's displacement amplitude
	struct NormalParams { UINT triCount; float dispAmp; UINT pad[2]; };

	D3D11_MAPPED_SUBRESOURCE mapped;
	context->Map(m_ParamsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	NormalParams* p = static_cast<NormalParams*>(mapped.pData);
	p->triCount = (UINT)(m_VertexCount / 3);
	p->dispAmp  = displacementAmplitude;
	p->pad[0]   = p->pad[1] = 0;
	context->Unmap(m_ParamsBuffer, 0);

	// 2. Set compute shader and resources, dispatch
	context->CSSetShader(m_ComputeShader, nullptr, 0);
	context->CSSetConstantBuffers(0, 1, &m_ParamsBuffer);
	context->CSSetShaderResources(0, 1, &m_InputSRV);
	context->CSSetUnorderedAccessViews(0, 1, &m_OutputUAV, nullptr);

	context->Dispatch(m_GroupCount, 1, 1);

	// 3. Unbind — a resource cannot be bound as UAV and SRV simultaneously.
	//    After unbinding the UAV, the output buffer is safe to use as SRV in VS.
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	ID3D11ShaderResourceView*  nullSRV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	context->CSSetShaderResources(0, 1, &nullSRV);
	context->CSSetShader(nullptr, nullptr, 0);
}

void NormalComputeClass::Shutdown()
{
	if (m_ParamsBuffer)   { m_ParamsBuffer->Release();   m_ParamsBuffer   = nullptr; }
	if (m_OutputSRV)      { m_OutputSRV->Release();      m_OutputSRV      = nullptr; }
	if (m_OutputUAV)      { m_OutputUAV->Release();      m_OutputUAV      = nullptr; }
	if (m_OutputBuffer)   { m_OutputBuffer->Release();   m_OutputBuffer   = nullptr; }
	if (m_ComputeShader)  { m_ComputeShader->Release();  m_ComputeShader  = nullptr; }
	// m_InputSRV is borrowed — ModelClass owns and releases it
}
