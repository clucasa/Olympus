cbuffer ViewProjMat	: register(c0)
{
    float4x4 ViewProj;
}

cbuffer ObjectMat	: register(c1)
{
    float4x4 viewInvProj;
	float4x4 viewPrev;

	float	 zNear;
	float	 zFar;
	float2	 padding;
}

Texture2D tex : register(t0);
Texture2D depth : register(t1);

struct VOut
{
	float4 posH : SV_POSITION;
    float2 texcoord : TEXCOORD;
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

	output.posH = mul(ViewProj, float4(position, 1.0f));
    output.texcoord = texcoord;

    return output;
}


float4 PShader(VOut input) : SV_TARGET
{
    float4 color = tex.Sample(samTriLinearSam, input.texcoord);
	color.r = 1.0f - color.r;
	color.g = 1.0f - color.g;
	color.b = 1.0f - color.b;
	color.a = 1.0f;	
	return color;
	//return float4(nearZ, farZ, 1, 1.0f);
}
