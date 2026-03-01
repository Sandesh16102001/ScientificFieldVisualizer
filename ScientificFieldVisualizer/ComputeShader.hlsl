// ComputeShader.hlsl  —  Per-face scalar smoothing pass
//
// The mesh is stored as a flat, pre-expanded triangle list:
//   vertex 0, 1, 2  →  triangle 0
//   vertex 3, 4, 5  →  triangle 1  ...
//
// Each compute thread handles one vertex.
// It finds the triangle that vertex belongs to (triBase = (i/3)*3),
// reads the original scalars of all 3 triangle corners, and writes
// their average to the output buffer.
//
// Result: the per-face-averaged scalars are used by the VertexShader
// instead of the raw per-vertex values, giving a smoother colormap
// with no sharp single-vertex spikes.

cbuffer SmoothParams : register(b0)
{
    uint vertexCount; // total expanded vertex count
    uint3 pad;
};

StructuredBuffer<float>   inputScalars  : register(t0); // original scalars (read-only)
RWStructuredBuffer<float> outputScalars : register(u0); // smoothed output (read-write)

[numthreads(64, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint i = DTid.x;
    if (i >= vertexCount)
        return; // extra threads from the last group — do nothing

    // Triangle this vertex belongs to starts at triBase
    uint triBase = (i / 3) * 3;

    // Average the scalar values of all 3 vertices in this triangle
    float avg = (inputScalars[triBase]
               + inputScalars[triBase + 1]
               + inputScalars[triBase + 2]) / 3.0f;

    outputScalars[i] = avg;
}
