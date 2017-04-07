//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

cbuffer cbPerObject
{
	float4x4 gWorldViewProj; 
};

struct VertexIn
{
    float4 PosL : POSITION;
    float4 Color : COLOR;
    float2 Size : SIZE;
};

struct VertexOut
{
	float4 PosL  : SV_POSITION;
    float4 Color : COLOR;
    float2 Size : SIZE;
};

struct GeoOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
	float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Just pass data over to geometry shader
	vout.PosL = vin.PosL;
	vout.Color = vin.Color;
    vout.Size = vin.Size;

    return vout;
}

float4 GetSideVertex(float2 size, int sides, int index, float PI)
{
    return float4(size.y * cos(2 * PI * index / sides), 0.0f, size.y * sin(2 * PI * index / sides), 1.0f);
}

[maxvertexcount(12)]
void GSThreeSides(point VertexOut gin[1], inout TriangleStream<GeoOut> triStream)
{
	float PI = 3.14;

	// create a new array for the vertices with that 
    // may not be declared in for loop since I get warnings if I do.. 
	float4 v[5];
    v[0] = float4(0.0f, gin[0].Size.x, 0.0f, 1.0f);
    v[1] = GetSideVertex(gin[0].Size, 3, 1, PI); 
    v[2] = GetSideVertex(gin[0].Size, 3, 2, PI);
    v[3] = GetSideVertex(gin[0].Size, 3, 3, PI);
    v[4] = float4(+0.0f, 0.0f, +0.0f, 1.0f);

	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    GeoOut gout[5];

    [unroll]
    for (int i = 0; i < 5; ++i)
    {
        // define side of pyramid
        gout[i].PosH = mul(v[i], gWorldViewProj);
        gout[i].PosW = v[i].xyz;
        gout[i].Color = color;
    }
    
    triStream.Append(gout[0]);
    triStream.Append(gout[2]);
    triStream.Append(gout[1]);
    triStream.Append(gout[4]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[1]);
    triStream.Append(gout[3]);
    triStream.Append(gout[4]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[3]);
    triStream.Append(gout[2]);
    triStream.Append(gout[4]);
    triStream.RestartStrip();
}

[maxvertexcount(16)]
void GSFourSides(point VertexOut gin[1], inout TriangleStream<GeoOut> triStream)
{
    float PI = 3.14;

	// create a new array for the vertices with that 
    // may not be declared in for loop since I get warnings if I do.. 
    float4 v[6];
    v[0] = float4(0.0f, gin[0].Size.x, 0.0f, 1.0f);
    v[1] = GetSideVertex(gin[0].Size, 4, 1, PI);
    v[2] = GetSideVertex(gin[0].Size, 4, 2, PI);
    v[3] = GetSideVertex(gin[0].Size, 4, 3, PI);
    v[4] = GetSideVertex(gin[0].Size, 4, 4, PI);
    v[5] = float4(+0.0f, 0.0f, +0.0f, 1.0f);

    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    GeoOut gout[6];

    [unroll]
    for (int i = 0; i < 6; ++i)
    {
        // define side of pyramid
        gout[i].PosH = mul(v[i], gWorldViewProj);
        gout[i].PosW = v[i].xyz;
        gout[i].Color = color;
    }
    
    triStream.Append(gout[0]);
    triStream.Append(gout[2]);
    triStream.Append(gout[1]);
    triStream.Append(gout[5]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[3]);
    triStream.Append(gout[2]);
    triStream.Append(gout[5]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[4]);
    triStream.Append(gout[3]);
    triStream.Append(gout[5]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[1]);
    triStream.Append(gout[4]);
    triStream.Append(gout[5]);
    triStream.RestartStrip();
}

[maxvertexcount(20)]
void GSFiveSides(point VertexOut gin[1], inout TriangleStream<GeoOut> triStream)
{
    float PI = 3.14;

	// create a new array for the vertices with that 
    // may not be declared in for loop since I get warnings if I do.. 
    float4 v[7];
    v[0] = float4(0.0f, gin[0].Size.x, 0.0f, 1.0f);
    v[1] = GetSideVertex(gin[0].Size, 5, 1, PI);
    v[2] = GetSideVertex(gin[0].Size, 5, 2, PI);
    v[3] = GetSideVertex(gin[0].Size, 5, 3, PI);
    v[4] = GetSideVertex(gin[0].Size, 5, 4, PI);
    v[5] = GetSideVertex(gin[0].Size, 5, 5, PI);
    v[6] = float4(+0.0f, 0.0f, +0.0f, 1.0f);

    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    GeoOut gout[7];

    [unroll]
    for (int i = 0; i < 7; ++i)
    {
        // define side of pyramid
        gout[i].PosH = mul(v[i], gWorldViewProj);
        gout[i].PosW = v[i].xyz;
        gout[i].Color = color;
    }
    
    triStream.Append(gout[0]);
    triStream.Append(gout[2]);
    triStream.Append(gout[1]);
    triStream.Append(gout[6]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[3]);
    triStream.Append(gout[2]);
    triStream.Append(gout[6]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[4]);
    triStream.Append(gout[3]);
    triStream.Append(gout[6]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[5]);
    triStream.Append(gout[4]);
    triStream.Append(gout[6]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[1]);
    triStream.Append(gout[5]);
    triStream.Append(gout[6]);
    triStream.RestartStrip();
}

[maxvertexcount(24)]
void GSSixSides(point VertexOut gin[1], inout TriangleStream<GeoOut> triStream)
{
    float PI = 3.14;

	// create a new array for the vertices with that 
    // may not be declared in for loop since I get warnings if I do.. 
    float4 v[8];
    v[0] = float4(0.0f, gin[0].Size.x, 0.0f, 1.0f);
    v[1] = GetSideVertex(gin[0].Size, 6, 1, PI);
    v[2] = GetSideVertex(gin[0].Size, 6, 2, PI);
    v[3] = GetSideVertex(gin[0].Size, 6, 3, PI);
    v[4] = GetSideVertex(gin[0].Size, 6, 4, PI);
    v[5] = GetSideVertex(gin[0].Size, 6, 5, PI);
    v[6] = GetSideVertex(gin[0].Size, 6, 6, PI);
    v[7] = float4(+0.0f, 0.0f, +0.0f, 1.0f);

    GeoOut gout[8];

    [unroll]
    for (int i = 0; i < 8; ++i)
    {
        // define side of pyramid
        gout[i].PosH = mul(v[i], gWorldViewProj);
        gout[i].PosW = v[i].xyz;
        gout[i].Color = gin[0].Color;
    }
    
    triStream.Append(gout[0]);
    triStream.Append(gout[2]);
    triStream.Append(gout[1]);
    triStream.Append(gout[7]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[3]);
    triStream.Append(gout[2]);
    triStream.Append(gout[7]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[4]);
    triStream.Append(gout[3]);
    triStream.Append(gout[7]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[5]);
    triStream.Append(gout[4]);
    triStream.Append(gout[7]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[6]);
    triStream.Append(gout[5]);
    triStream.Append(gout[7]);
    triStream.RestartStrip();

    triStream.Append(gout[0]);
    triStream.Append(gout[1]);
    triStream.Append(gout[6]);
    triStream.Append(gout[7]);
    triStream.RestartStrip();
}

float4 PS(GeoOut pin) : SV_Target
{
    return pin.Color;
}

technique11 ThreeSideTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader(CompileShader(gs_5_0, GSThreeSides()));
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

technique11 FourSideTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GSFourSides()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}

technique11 FiveSideTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GSFiveSides()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}

technique11 SixSideTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GSSixSides()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}