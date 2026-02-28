#include "ModelClass.h"

ModelClass::ModelClass()
{
	m_VertexBuffer = nullptr;
	m_IndexBuffer = nullptr;

	m_VertexCount = 0;
	m_IndexCount = 0;
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
	delete[] vertices;
	if (FAILED(hr)) return false;

	return true;
}

void ModelClass::ShutdownBuffers()
{
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
