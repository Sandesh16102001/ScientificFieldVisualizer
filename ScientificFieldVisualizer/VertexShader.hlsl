cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
    float4 position    : POSITION;
    float  scalarValue : TEXCOORD0;
    float3 normal      : NORMAL;
    float3 barycentric : TEXCOORD2;
};

struct PixelInputType
{
    float4 position    : SV_POSITION;
    float  scalarValue : TEXCOORD0;
    float3 normal      : TEXCOORD1;
    float3 barycentric : TEXCOORD2;
};

PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.scalarValue = input.scalarValue;
    output.normal      = mul(float4(input.normal, 0.0f), worldMatrix).xyz;
    output.barycentric = input.barycentric;

    return output;
}
