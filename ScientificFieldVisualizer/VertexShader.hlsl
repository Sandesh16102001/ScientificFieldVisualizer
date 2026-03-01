cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    float  displacementAmplitude; // How much to extrude along displacement vector
    float3 pad;                   // pad to 16 bytes
};

// Smoothed scalar values written by the scalar-smoothing compute shader (ComputeShader.hlsl).
// Indexed by SV_VertexID so each vertex reads its own post-smoothed value.
StructuredBuffer<float>  smoothedScalars   : register(t0);

// Recomputed face normals written by the normal-recompute compute shader (NormalComputeShader.hlsl).
// Corrects stale sphere normals after GPU displacement deformation.
StructuredBuffer<float3> recomputedNormals : register(t1);

struct VertexInputType
{
    float4 position     : POSITION;
    float  scalarValue  : TEXCOORD0;  // original value (kept in vertex buffer; unused here)
    float3 normal       : NORMAL;
    float3 barycentric  : TEXCOORD2;
    float3 displacement : TEXCOORD3;
};

struct PixelInputType
{
    float4 position    : SV_POSITION;
    float  scalarValue : TEXCOORD0;  // smoothed (face-averaged) scalar — used for colormap
    float3 normal      : TEXCOORD1;
    float3 barycentric : TEXCOORD2;
    float3 worldPos    : TEXCOORD3;  // world-space position for specular view direction
    float  rawScalar   : TEXCOORD4;  // original per-vertex scalar — used for isolines
};

PixelInputType ColorVertexShader(VertexInputType input, uint vertexID : SV_VertexID)
{
    PixelInputType output;

    input.position.w = 1.0f;

    // --- GPU Displacement Deformation ---
    // Push the vertex outward along the pre-calculated displacement vector
    // The CPU controls the 'amplitude' live via the constant buffer
    input.position.xyz += input.displacement * displacementAmplitude;

    // Compute world-space position once and reuse for both clip-space and worldPos output
    float4 worldPos4   = mul(input.position, worldMatrix);
    output.worldPos    = worldPos4.xyz;
    output.position    = mul(worldPos4, viewMatrix);
    output.position    = mul(output.position, projectionMatrix);

    // Read compute-shader-smoothed scalar (t0) — colormap uses this
    output.scalarValue = smoothedScalars[vertexID];
    // Original per-vertex scalar from vertex buffer — isolines use this so fwidth has a real gradient
    output.rawScalar   = input.scalarValue;
    // Read post-deformation face normal recomputed by NormalComputeShader (t1)
    output.normal      = mul(float4(recomputedNormals[vertexID], 0.0f), worldMatrix).xyz;
    output.barycentric = input.barycentric;

    return output;
}
