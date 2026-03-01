#include "ShaderClass.h"

ShaderClass::ShaderClass()
{
	m_VertexShader		= nullptr;
	m_PixelShader		= nullptr;
	m_InputLayout		= nullptr;
	m_MatrixBuffer		= nullptr;
	m_ShaderParamsBuffer= nullptr;
}

ShaderClass::~ShaderClass()
{
}

bool ShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{

	bool result;
	wchar_t vsFileName[128];
	wchar_t psFileName[128];
	int error;
	
	error = wcscpy_s(vsFileName, 128, L"VertexShader.hlsl");
	if (error != 0)
		return false;

	error = wcscpy_s(psFileName, 128, L"PixelShader.hlsl");
	if (error != 0)
		return false;

	result = InitializeShader(device, hwnd, vsFileName, psFileName);
	if (!result)
	{
		return false;
	}

	return true;
}

void ShaderClass::Shutdown()
{
	ShutdownShader();
}

bool ShaderClass::Render(ID3D11DeviceContext* context, int indexCount, const DrawParams& params)
{
	bool result = SetShaderParameters(context, params);
	if (!result)
		return false;

	RenderShader(context, indexCount);
	return true;
}

bool ShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd , WCHAR* vsfilename, WCHAR* psfilename)
{

	HRESULT						hr;
	ID3D10Blob*					errorMessage;
	ID3D10Blob*					vertexShaderBuffer;
	ID3D10Blob*					pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC	polygonLayout[5];
	unsigned int				numElements;
	D3D11_BUFFER_DESC			matrixBufferDesc;


	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;



	hr = D3DCompileFromFile(vsfilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);


	if (FAILED(hr))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsfilename);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, vsfilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}


	// Compile the pixel shader code.
	hr = D3DCompileFromFile(psfilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	if (FAILED(hr))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psfilename);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, psfilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}


	// Creating the vertex shader from the buffer.
	hr = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_VertexShader);
	if (FAILED(hr))
	{
		return false;
	}

	// Creating the pixel shader from the buffer.
	hr = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_PixelShader);
	if (FAILED(hr))
	{
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	polygonLayout[3].SemanticName = "TEXCOORD";
	polygonLayout[3].SemanticIndex = 2;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "TEXCOORD";
	polygonLayout[4].SemanticIndex = 3;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[4].InputSlot = 0;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[4].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	hr = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_InputLayout);
	if (FAILED(hr))
	{
		return false;
	}

	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;


	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferTye);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the VS matrix constant buffer.
	hr = device->CreateBuffer(&matrixBufferDesc, NULL, &m_MatrixBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	// Create the PS shader-params constant buffer (colormapIndex etc.).
	D3D11_BUFFER_DESC shaderParamsDesc;
	shaderParamsDesc.Usage				= D3D11_USAGE_DYNAMIC;
	shaderParamsDesc.ByteWidth			= sizeof(ShaderParamsType);
	shaderParamsDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	shaderParamsDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	shaderParamsDesc.MiscFlags			= 0;
	shaderParamsDesc.StructureByteStride= 0;

	hr = device->CreateBuffer(&shaderParamsDesc, NULL, &m_ShaderParamsBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

void ShaderClass::ShutdownShader()
{

	// Release the shader-params buffer.
	if (m_ShaderParamsBuffer)
	{
		m_ShaderParamsBuffer->Release();
		m_ShaderParamsBuffer = nullptr;
	}

	// Release the matrix constant buffer.
	if (m_MatrixBuffer)
	{
		m_MatrixBuffer->Release();
		m_MatrixBuffer = nullptr;
	}

	// Release the layout.
	if (m_InputLayout)
	{
		m_InputLayout->Release();
		m_InputLayout = nullptr;
	}

	// Release the pixel shader.
	if (m_PixelShader)
	{
		m_PixelShader->Release();
		m_PixelShader = nullptr;
	}

	// Release the vertex shader.
	if (m_VertexShader)
	{
		m_VertexShader->Release();
		m_VertexShader = nullptr;
	}

}

void ShaderClass::OutputShaderErrorMessage(ID3D10Blob* errormsg, HWND hwnd, WCHAR* shaderfilename)
{
	char* compileErrors;
	unsigned long long bufferSize, i;
	std::ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errormsg->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errormsg->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errormsg->Release();
	errormsg = nullptr;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderfilename, MB_OK);

}

bool ShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, const DrawParams& params)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferTye* dataPtr;

	XMMATRIX world = XMMatrixTranspose(params.world);
	XMMATRIX view  = XMMatrixTranspose(params.view);
	XMMATRIX proj  = XMMatrixTranspose(params.proj);

	// --- Update VS matrix constant buffer ---
	hr = deviceContext->Map(m_MatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
		return false;

	dataPtr = (MatrixBufferTye*)mappedResource.pData;
	dataPtr->world                = world;
	dataPtr->view                 = view;
	dataPtr->projection           = proj;
	dataPtr->displacementAmplitude = params.displacementAmplitude;
	dataPtr->pad[0] = dataPtr->pad[1] = dataPtr->pad[2] = 0.0f;

	deviceContext->Unmap(m_MatrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_MatrixBuffer);

	// Bind both VS structured buffer SRVs in a single call:
	//   t0 = smoothedScalars   (from ComputeShaderClass output)
	//   t1 = recomputedNormals (from NormalComputeClass output)
	ID3D11ShaderResourceView* vsSrvs[2] = { params.scalarSRV, params.normalSRV };
	deviceContext->VSSetShaderResources(0, 2, vsSrvs);

	// --- Update PS shader-params constant buffer ---
	D3D11_MAPPED_SUBRESOURCE mappedParams;
	hr = deviceContext->Map(m_ShaderParamsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedParams);
	if (FAILED(hr))
		return false;

	ShaderParamsType* paramsPtr = (ShaderParamsType*)mappedParams.pData;
	paramsPtr->colormapIndex   = params.colormapIndex;
	paramsPtr->wireframeOn     = params.wireframeOn;
	paramsPtr->isolineOn       = params.isolineOn;
	paramsPtr->pad             = 0;
	paramsPtr->scalarMin       = params.scalarMin;
	paramsPtr->scalarMax       = params.scalarMax;
	paramsPtr->isolineInterval = params.isolineInterval;
	paramsPtr->pad2            = 0.0f;
	paramsPtr->cameraPos       = params.cameraPos;
	paramsPtr->pad3            = 0.0f;

	deviceContext->Unmap(m_ShaderParamsBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &m_ShaderParamsBuffer);

	return true;
}

void ShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_InputLayout);
	deviceContext->VSSetShader(m_VertexShader, NULL, 0);
	deviceContext->PSSetShader(m_PixelShader, NULL, 0);

	deviceContext->Draw(indexCount, 0);   // non-indexed — vertices pre-expanded

}

