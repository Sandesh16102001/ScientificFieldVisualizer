#pragma once
#include<d3d11.h>
#include<DirectXMath.h>
using namespace DirectX;


class ModelClass
{

private : 
	struct VertexType
	{
		XMFLOAT3 position;
		float    scalarValue;
		XMFLOAT3 normal;
		XMFLOAT3 barycentric;
		XMFLOAT3 displacement;
	};

public: 
	ModelClass();
	~ModelClass();

	bool Initialize(ID3D11Device* device);
	void ShutDown();
	void Render(ID3D11DeviceContext* deviceContext);


	int GetIndexCount();

	// SRV over original per-vertex scalar values -- input to ComputeShaderClass.
	ID3D11ShaderResourceView* GetScalarSRV()  const { return m_ScalarSRV; }

	// SRV over per-vertex {position, displacement} pairs -- input to NormalComputeClass.
	ID3D11ShaderResourceView* GetPosDispSRV() const { return m_PosDispSRV; }


private : 

	bool InitializeBuffers(ID3D11Device* device );
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

private : 
	
	ID3D11Buffer*			  m_VertexBuffer;
	ID3D11Buffer*			  m_IndexBuffer;

	// Structured buffer of floats (one per expanded vertex) -- scalar smoothing CS input.
	ID3D11Buffer*			  m_ScalarBuffer;
	ID3D11ShaderResourceView* m_ScalarSRV;

	// Structured buffer of {float3 position, float3 displacement} -- normal CS input.
	ID3D11Buffer*			  m_PosDispBuffer;
	ID3D11ShaderResourceView* m_PosDispSRV;

	int					 m_VertexCount;
	int				 	 m_IndexCount;



};