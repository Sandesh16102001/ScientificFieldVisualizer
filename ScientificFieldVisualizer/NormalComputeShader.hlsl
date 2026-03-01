// NormalComputeShader.hlsl  —  Per-face normal recomputation after GPU displacement.
//
// WHY THIS PASS EXISTS:
//   The vertex shader deforms the mesh by adding (displacement * amplitude) to
//   every vertex position. The original sphere normals stored in the vertex
//   buffer then point in the wrong direction, causing incorrect Lambertian
//   shading at non-zero deformation values.
//
//   This compute pass runs every frame BEFORE the draw call.
//   Each thread handles ONE TRIANGLE (three consecutive expanded vertices).
//   It reads the displaced positions of the three corners, computes the cross
//   product of two edges, and assigns that face normal to all three vertices.
//
// DATA FLOW:
//   inputVerts (pos+disp per vertex)  →  [cross product]  →  outputNormals
//   outputNormals is then bound to VS register t1 each draw call.

cbuffer NormalParams : register(b0)
{
    uint  triangleCount;          // total triangles  =  vertexCount / 3
    float displacementAmplitude;  // must match the value sent to the vertex shader
    uint2 pad;                    // pad to 16 bytes
};

// Each element stores the position and displacement of one expanded vertex.
// Stride = 24 bytes (two float3 tightly packed — structured buffers do not
// apply the 16-byte constant-buffer padding rule).
struct PosDisp
{
    float3 position;
    float3 displacement;
};

StructuredBuffer<PosDisp>   inputVerts    : register(t0); // read-only input
RWStructuredBuffer<float3>  outputNormals : register(u0); // write output

[numthreads(64, 1, 1)]
void CSNormals(uint3 DTid : SV_DispatchThreadID)
{
    uint triIdx = DTid.x;
    if (triIdx >= triangleCount)
        return; // extra threads past the last triangle — do nothing

    uint base = triIdx * 3;

    // Apply the same displacement the vertex shader will apply this frame
    float3 p0 = inputVerts[base + 0].position + inputVerts[base + 0].displacement * displacementAmplitude;
    float3 p1 = inputVerts[base + 1].position + inputVerts[base + 1].displacement * displacementAmplitude;
    float3 p2 = inputVerts[base + 2].position + inputVerts[base + 2].displacement * displacementAmplitude;

    // Two edge vectors of the deformed triangle
    float3 edge0 = p1 - p0;
    float3 edge1 = p2 - p0;

    // Face normal via cross product (right-hand rule matches the winding order
    // used when the sphere quads were split into tl-bl-tr / tr-bl-br pairs)
    float3 faceNormal = normalize(cross(edge0, edge1));

    // All three vertices of this triangle share the same face normal
    outputNormals[base + 0] = faceNormal;
    outputNormals[base + 1] = faceNormal;
    outputNormals[base + 2] = faceNormal;
}
