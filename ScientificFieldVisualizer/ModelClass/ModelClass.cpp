#include "ModelClass.h"

ModelClass::ModelClass()
{
	m_VertexBuffer  = nullptr;
	m_IndexBuffer   = nullptr;
	m_ScalarBuffer  = nullptr;
	m_ScalarSRV     = nullptr;
	m_PosDispBuffer = nullptr;
	m_PosDispSRV    = nullptr;

	m_VertexCount = 0;
	m_IndexCount  = 0;
}

ModelClass::~ModelClass()
{
}

bool ModelClass::Initialize(ID3D11Device* device)
{
	bool result;

	result = InitializeBuffers(device);
	if (!result)
		return false;

	return true;
}

void ModelClass::ShutDown()
{
	ShutdownBuffers();
}

void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	RenderBuffers(deviceContext);
}

int ModelClass::GetIndexCount()
{
	return m_IndexCount;
}

bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC       vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA  vertexData;
	HRESULT                 hr;

	// UV-sphere parameters: 80 stacks x 80 slices = 12,800 triangles
	const int stacks   = 80;
	const int slices   = 80;
	const int numRows  = stacks + 1;
	const int numCols  = slices + 1;
	const int numTris  = stacks * slices * 2;

	// ------------------------------------------------------------------
	// Step 1 — Generate sphere into indexed temp arrays
	// ------------------------------------------------------------------
	int tempVertCount = numRows * numCols;
	VertexType* tempVerts = new VertexType[tempVertCount];
	if (!tempVerts) return false;

	const float PI = 3.14159265358979323846f;
	int vIdx = 0;
	for (int i = 0; i < numRows; i++)
	{
		float theta = PI * (float)i / (float)stacks;
		for (int j = 0; j < numCols; j++)
		{
			float phi = 2.0f * PI * (float)j / (float)slices;
			float x = sinf(theta) * cosf(phi);
			float y = cosf(theta);
			float z = sinf(theta) * sinf(phi);

			tempVerts[vIdx].position    = XMFLOAT3(x, y, z);
			tempVerts[vIdx].scalarValue = sinf(x) * cosf(z) * 0.5f + 0.5f;
			tempVerts[vIdx].normal      = XMFLOAT3(x, y, z);
			// Displacement is outward along the normal, scaled by the scalar field itself
			tempVerts[vIdx].displacement = XMFLOAT3(x * tempVerts[vIdx].scalarValue, y * tempVerts[vIdx].scalarValue, z * tempVerts[vIdx].scalarValue);
			tempVerts[vIdx].barycentric = XMFLOAT3(0, 0, 0); // filled during expansion
			vIdx++;
		}
	}

	int tempIdxCount = numTris * 3;
	unsigned long* tempIndices = new unsigned long[tempIdxCount];
	if (!tempIndices) { delete[] tempVerts; return false; }

	int iIdx = 0;
	for (int i = 0; i < stacks; i++)
	{
		for (int j = 0; j < slices; j++)
		{
			unsigned long tl = (unsigned long)(i * numCols + j);
			unsigned long tr = tl + 1;
			unsigned long bl = tl + numCols;
			unsigned long br = bl + 1;

			tempIndices[iIdx++] = tl; tempIndices[iIdx++] = bl; tempIndices[iIdx++] = tr;
			tempIndices[iIdx++] = tr; tempIndices[iIdx++] = bl; tempIndices[iIdx++] = br;
		}
	}

	// ------------------------------------------------------------------
	// Step 2 — Expand to non-indexed vertices with barycentric coords
	// Each triangle gets 3 unique vertices: (1,0,0), (0,1,0), (0,0,1)
	// ------------------------------------------------------------------
	m_VertexCount = numTris * 3;
	m_IndexCount  = m_VertexCount; // Draw() uses vertex count, not index count

	VertexType* vertices = new VertexType[m_VertexCount];
	if (!vertices) { delete[] tempVerts; delete[] tempIndices; return false; }

	XMFLOAT3 bary[3] = { {1.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f}, {0.0f,0.0f,1.0f} };
	for (int t = 0; t < numTris; t++)
	{
		for (int k = 0; k < 3; k++)
		{
			vertices[t * 3 + k]             = tempVerts[tempIndices[t * 3 + k]];
			vertices[t * 3 + k].barycentric = bary[k];
		}
	}

	delete[] tempVerts;
	delete[] tempIndices;

	// ------------------------------------------------------------------
	// Step 3 — Create vertex buffer (no index buffer — use Draw())
	// ------------------------------------------------------------------
	vertexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth           = sizeof(VertexType) * m_VertexCount;
	vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags      = 0;
	vertexBufferDesc.MiscFlags           = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem          = vertices;
	vertexData.SysMemPitch      = 0;
	vertexData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_VertexBuffer);
	if (FAILED(hr)) { delete[] vertices; return false; }

	// ------------------------------------------------------------------
	// Step 4 — Extract auxiliary data before freeing the vertex array
	// Both the scalar smoothing CS and the normal recompute CS need
	// separate structured buffers sourced from the expanded vertex data.
	// ------------------------------------------------------------------
	struct PosDispData { XMFLOAT3 position; XMFLOAT3 displacement; }; // 24 bytes

	float*       scalars  = new float[m_VertexCount];
	PosDispData* posDisp  = new PosDispData[m_VertexCount];
	if (!scalars || !posDisp)
	{
		delete[] vertices;
		delete[] scalars;
		delete[] posDisp;
		return false;
	}

	for (int i = 0; i < m_VertexCount; i++)
	{
		scalars[i]             = vertices[i].scalarValue;
		posDisp[i].position    = vertices[i].position;
		posDisp[i].displacement = vertices[i].displacement;
	}

	delete[] vertices;

	// ------------------------------------------------------------------
	// Step 4a — Scalar structured buffer + SRV (ComputeShaderClass input)
	// ------------------------------------------------------------------
	D3D11_BUFFER_DESC scalarDesc        = {};
	scalarDesc.ByteWidth                = sizeof(float) * m_VertexCount;
	scalarDesc.Usage                    = D3D11_USAGE_DEFAULT;
	scalarDesc.BindFlags                = D3D11_BIND_SHADER_RESOURCE;
	scalarDesc.MiscFlags                = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	scalarDesc.StructureByteStride      = sizeof(float);

	D3D11_SUBRESOURCE_DATA scalarData   = {};
	scalarData.pSysMem                  = scalars;

	hr = device->CreateBuffer(&scalarDesc, &scalarData, &m_ScalarBuffer);
	delete[] scalars;
	if (FAILED(hr)) { delete[] posDisp; return false; }

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format                   = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension            = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.NumElements       = (UINT)m_VertexCount;

	hr = device->CreateShaderResourceView(m_ScalarBuffer, &srvDesc, &m_ScalarSRV);
	if (FAILED(hr)) { delete[] posDisp; return false; }

	// ------------------------------------------------------------------
	// Step 4b — PosDisp structured buffer + SRV (NormalComputeClass input)
	// Each element: { float3 position, float3 displacement } = 24 bytes
	// NormalComputeShader reads these to reconstruct deformed positions
	// and compute the correct post-displacement face normal.
	// ------------------------------------------------------------------
	D3D11_BUFFER_DESC pdDesc         = {};
	pdDesc.ByteWidth                 = sizeof(PosDispData) * m_VertexCount;
	pdDesc.Usage                     = D3D11_USAGE_DEFAULT;
	pdDesc.BindFlags                 = D3D11_BIND_SHADER_RESOURCE;
	pdDesc.MiscFlags                 = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	pdDesc.StructureByteStride       = sizeof(PosDispData); // 24

	D3D11_SUBRESOURCE_DATA pdData    = {};
	pdData.pSysMem                   = posDisp;

	hr = device->CreateBuffer(&pdDesc, &pdData, &m_PosDispBuffer);
	delete[] posDisp;
	if (FAILED(hr)) return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC pdSrvDesc = {};
	pdSrvDesc.Format                   = DXGI_FORMAT_UNKNOWN;
	pdSrvDesc.ViewDimension            = D3D11_SRV_DIMENSION_BUFFER;
	pdSrvDesc.Buffer.NumElements       = (UINT)m_VertexCount;

	hr = device->CreateShaderResourceView(m_PosDispBuffer, &pdSrvDesc, &m_PosDispSRV);
	if (FAILED(hr)) return false;

	return true;
}

void ModelClass::ShutdownBuffers()
{
	if (m_PosDispSRV)
	{
		m_PosDispSRV->Release();
		m_PosDispSRV = nullptr;
	}

	if (m_PosDispBuffer)
	{
		m_PosDispBuffer->Release();
		m_PosDispBuffer = nullptr;
	}

	if (m_ScalarSRV)
	{
		m_ScalarSRV->Release();
		m_ScalarSRV = nullptr;
	}

	if (m_ScalarBuffer)
	{
		m_ScalarBuffer->Release();
		m_ScalarBuffer = nullptr;
	}

	if (m_IndexBuffer)
	{
		m_IndexBuffer->Release();
		m_IndexBuffer = nullptr;
	}

	if (m_VertexBuffer)
	{
		m_VertexBuffer->Release();
		m_VertexBuffer = nullptr;
	}
}

void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	// No index buffer: vertices are pre-expanded per-triangle for barycentric support
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
