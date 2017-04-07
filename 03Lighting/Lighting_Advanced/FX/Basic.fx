//=============================================================================
// Basic.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Basic effect that currently supports transformations, lighting, and texturing.
//=============================================================================

#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
    DirectionalLight gDirLights[3];
	float3 gEyePosW;

    float gFogStart;
    float gFogRange;
    float4 gFogColor;

    PointLight gPointLight;
    float offset;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gWorldViewProj2;
	float4x4 gTexTransform;
	Material gMaterial;
}; 

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap;
Texture2D gProjectionTexture;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float4 ViewPosW    : LIGHT_POSITION;
    float3 NormalW : NORMAL;
	float2 Tex     : TEXCOORD;
    float3 LightPos : TEXCOORD2;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to world space space.
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    vout.ViewPosW = mul(float4(vin.PosL, 1.0f), gWorldViewProj2);

	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

    vout.LightPos = gPointLight.Position.xyz - vout.PosW.xyz;

	return vout;
}
 
float4 PS(VertexOut pin, uniform int gLightCount, uniform bool gUseTexure) : SV_Target
{
    float brightness = 4.0f;
    float lightIntensity;

    float4 color;
    float4 texColor = float4(1, 1, 1, 1);
    float4 projectionColor;
    float4 projectionColor2;

    float2 projectTexCoord;
    float2 projectTexCoord2;

	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
	
    // out of range of the light
    if ((pin.LightPos.x * pin.LightPos.x + pin.LightPos.z * pin.LightPos.z + pin.LightPos.y * pin.LightPos.y) > (gPointLight.Range * gPointLight.Range))
    {
        lightIntensity = 0.0f;
    }
    else
    {
        lightIntensity = saturate(dot(pin.NormalW, pin.LightPos)); 
    }

    if (lightIntensity > 0.0f)
    {
        // Determine the light color based on the diffuse color and the amount of light intensity.
        color = (gPointLight.Diffuse * gPointLight.Ambient * lightIntensity) * brightness;
    }

    // Default to multiplicative identity.
    if(gUseTexure)
	{
		// Sample texture.
		texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex );
	}

	//
	// Lighting.
	//

	float4 litColor = texColor;
	if( gLightCount > 0  )
	{  
		// Start with a sum of zero. 
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

        float4 A, D, S;

		// Sum the light contribution from each light source.  
		[unroll]
		for(int i = 0; i < gLightCount; ++i)
		{
			ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, 
				A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}

		// Modulate with late add.
		litColor = texColor*(ambient + diffuse) + spec;
	}

    if (true)
    {
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

		// Blend the fog color and the lit color.
        litColor = lerp(litColor, gFogColor, fogLerp);
    }

    if (lightIntensity > 0.0f)
    {
        projectTexCoord.x = pin.ViewPosW.x / pin.ViewPosW.w / 2.0f + 0.5f + offset;
        projectTexCoord.y = -pin.ViewPosW.y / pin.ViewPosW.w / 2.0f + 0.5f;

        projectTexCoord2.x = pin.ViewPosW.x / pin.ViewPosW.w / 2.0f + 0.5f;
        projectTexCoord2.y = projectTexCoord.y - offset + 0.03;

        if (projectTexCoord.x < 0)
        {
            projectTexCoord.x = 1 + projectTexCoord.x;
        }
        else if (projectTexCoord.x > 1.0f)
        {
            projectTexCoord.x = projectTexCoord.x - 1;
        }

        if (projectTexCoord2.y < 0)
        {
            projectTexCoord2.y = 1 + projectTexCoord2.y;
        }
        else if (projectTexCoord2.y > 1.0f)
        {
            projectTexCoord2.y = projectTexCoord2.y - 1;
        }

        projectionColor = gProjectionTexture.Sample(samAnisotropic, projectTexCoord);
        projectionColor2 = gProjectionTexture.Sample(samAnisotropic, projectTexCoord2);

        litColor = saturate((color * projectionColor * projectionColor2 * litColor) + (gPointLight.Ambient * 2 * litColor));
    }
    else
    {
        litColor = gPointLight.Ambient * litColor;
    }

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

technique11 Light1
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, false) ) );
    }
}

technique11 Light2
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, false) ) );
    }
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false) ) );
    }
}

technique11 Light0Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(0, true) ) );
    }
}

technique11 Light1Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, true) ) );
    }
}

technique11 Light2Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, true) ) );
    }
}

technique11 Light3Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true) ) );
    }
}
