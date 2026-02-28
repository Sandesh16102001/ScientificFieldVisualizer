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
		float     scalarValue;
		XMFLOAT3 normal;
		XMFLOAT3 barycentric;
	};

public: 
	ModelClass();
	~ModelClass();

	bool Initialize(ID3D11Device* device);
	void ShutDown();
	void Render(ID3D11DeviceContext* deviceContext);


	int GetIndexCount();


private : 

	bool InitializeBuffers(ID3D11Device* device );
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

private : 
	
	ID3D11Buffer*		 m_VertexBuffer;
	ID3D11Buffer*		 m_IndexBuffer;

	int					 m_VertexCount;
	int				 	 m_IndexCount;



};