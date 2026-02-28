cbuffer ShaderParamBuffer : register(b0)
{
    int   colormapIndex; // 0 = Viridis, 1 = Jet
    int   wireframeOn;   // 0 = off,     1 = on
    float scalarMin;     // visible range lower bound [0..1]
    float scalarMax;     // visible range upper bound [0..1]
};

struct PixelInputType
{
    float4 position    : SV_POSITION;
    float  scalarValue : TEXCOORD0;
    float3 normal      : TEXCOORD1;
    float3 barycentric : TEXCOORD2;
};

// ---------------------------------------------------------------------------
// Viridis colormap — degree-6 polynomial (Matplotlib coefficients)
// ---------------------------------------------------------------------------
float3 Viridis(float t)
{
    const float3 c0 = float3( 0.2777273272234177,  0.005407344544966578,  0.3340998053353061);
    const float3 c1 = float3( 0.1050930431085774,  1.404613529898575,     1.384590162594685 );
    const float3 c2 = float3(-0.3308618287255563,  0.214847559468213,     0.09509516302823659);
    const float3 c3 = float3(-4.634230498983486,  -5.799100973351585,   -19.33244095627987  );
    const float3 c4 = float3( 6.228269936347081,  14.17993336680509,     56.69055260068105  );
    const float3 c5 = float3( 4.776384997670288, -13.74514537774601,    -65.35303263337234  );
    const float3 c6 = float3(-5.435455855934631,   4.645852612178535,    26.3124352495832   );
    return saturate(c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6))))));
}

// ---------------------------------------------------------------------------
// Jet colormap — piecewise linear (blue -> cyan -> green -> yellow -> red)
// ---------------------------------------------------------------------------
float3 Jet(float t)
{
    float r = saturate(1.5f - abs(4.0f * t - 3.0f));
    float g = saturate(1.5f - abs(4.0f * t - 2.0f));
    float b = saturate(1.5f - abs(4.0f * t - 1.0f));
    return float3(r, g, b);
}

float4 ColorPixelShader(PixelInputType input) : SV_TARGET
{
    // --- Lambertian Diffuse Lighting ---
    float3 lightDir  = normalize(float3(1.0f, 1.0f, -1.0f));
    float3 N         = normalize(input.normal);
    float  diffuse   = max(0.0f, dot(N, lightDir));
    float  ambient   = 0.15f;
    float  brightness = ambient + (1.0f - ambient) * diffuse;

    float  t = saturate(input.scalarValue);

    // --- Scalar Range Clamp ---
    // Values outside [scalarMin, scalarMax] are shown in a distinct grey.
    float3 outOfRangeColor = float3(0.25f, 0.25f, 0.25f); // dark grey
    float3 color;

    float span = scalarMax - scalarMin;
    if (t < scalarMin || t > scalarMax)
    {
        // Out-of-range: show clearly separate grey (no lighting, always visible)
        return float4(outOfRangeColor, 1.0f);
    }
    else
    {
        // Remap [scalarMin, scalarMax] -> [0, 1] for the colormap
        float normalized = (span > 0.001f) ? (t - scalarMin) / span : 0.5f;
        color = (colormapIndex == 0) ? Viridis(normalized) : Jet(normalized);
    }

    float4 finalColor = float4(color * brightness, 1.0f);

    // --- Wireframe overlay ---
    if (wireframeOn)
    {
        float3 bary  = input.barycentric;
        float3 db    = max(fwidth(bary), 0.0001f);
        float3 edge  = smoothstep(float3(0,0,0), db * 1.5f, bary);
        float  edgeFactor = min(edge.x, min(edge.y, edge.z));
        float3 wireColor  = float3(0.9f, 0.9f, 0.9f);
        finalColor = float4(lerp(wireColor, finalColor.rgb, edgeFactor), 1.0f);
    }

    return finalColor;
}
