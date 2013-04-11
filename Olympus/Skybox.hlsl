cbuffer ViewProjMat	: c0
{
    float4x4 ViewProj;
}

cbuffer ObjectMat	: c1
{
    float4x4 matFinal;
}


TextureCube cubeMap;

struct VOut
{
    float4 posH : SV_POSITION;
    float3 posL : POSITION;
};

SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

VOut VShader(float3 position : POSITION, float3 normal : NORMAL, float3 tangent : TANGENT, float2 texcoord : TEXCOORD )
{
    VOut output;

    output.posH = mul(mul(ViewProj, matFinal), float4(position, 1.0f)).xyww;    // transform the vertex from 3D to 2D
    output.posL = position;

    return output;
}


float4 PShader(VOut input) : SV_TARGET
{
    float4 color =  cubeMap.Sample(samTriLinearSam, input.posL);
    color.a = 1.0f;
    return color;
}
