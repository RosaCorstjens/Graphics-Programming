//=============================================================================
// Lighting.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and lights geometry.
//=============================================================================

// https://www.shadertoy.com/view/MdfcRS#

#include "LightHelper.fx"
 
#define EDGE_WIDTH 0.5
#define SMOOTH 0.025
#define ITERATIONS 40
#define FAR 40

cbuffer cbPerFrame
{
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	float3 gEyePosW;

    float2 gResolution;
    float gGlobalTime;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	Material gMaterial;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
};

// ------------------------------------------------------------------------------------------------

float hash(float n)
{
    return frac(sin(n) * 43758.5453);
}

float hash3(float3 n)
{
    return frac(sin(1000 * dot(n, float3(1, 57, -13.7))) * 4375.5453);
}

float noise3(float3 x)
{
    float3 p = floor(x);
    float3 f = frac(x);

    f = f * f * (3.0f - 2.0f * f);              // to make derivative continuous at borders
    
    return lerp(
        lerp(lerp(hash3(p + float3(0, 0, 0)), hash3(p + float3(1, 0, 0)), f.x), // triilinear interp
             lerp(hash3(p + float3(0, 1, 0)), hash3(p + float3(1, 1, 0)), f.x), f.y
    ),
        lerp(lerp(hash3(p + float3(0, 0, 1)), hash3(p + float3(1, 0, 1)), f.x),
             lerp(hash3(p + float3(0, 1, 1)), hash3(p + float3(1, 1, 1)), f.x), f.y), f.z
    );
}
float noise(float3 x)
{
    return (noise3(x) + noise3(x + 11.5f)) / 2.0f;
}

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to world space space.
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
		
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	return vout;
}
  
float4 PS(VertexOut pin) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW); 

	float3 toEyeW = normalize(gEyePosW - pin.PosW);

	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the light contribution from each light source.
	float4 A, D, S;

	ComputeDirectionalLight(gMaterial, gDirLight, pin.NormalW, toEyeW, A, D, S);
	ambient += A;  
	diffuse += D;
	spec    += S;

	ComputePointLight(gMaterial, gPointLight, pin.PosW, pin.NormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec    += S;

	ComputeSpotLight(gMaterial, gSpotLight, pin.PosW, pin.NormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec    += S;
	   
	float4 litColor = ambient + diffuse + spec;

	// Common to take alpha from diffuse material.
	litColor.a = gMaterial.Diffuse.a;

    return litColor;
}

// Noise pixel shader
float4 PSNoise(VertexOut pin) : SV_Target
{
    pin.PosW *= 8.0f / gResolution.y;
    float n = noise(float3(pin.PosW.xy, 0.1f * gGlobalTime));
    float v = sin(6.28f * 10.0f * n);
    v = smoothstep(0.0f, 1.0f, 0.7f * abs(v) / fwidth(v));
    n = floor(n * 20.0f) / 20.0f;

    return v * (0.5f + 0.5f * cos(12.0f * n + float4(0.0f, 2.1f, -2.1f, 0.0f)));
}

technique11 LightTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

technique11 NoiseTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSNoise()));
    }
}
