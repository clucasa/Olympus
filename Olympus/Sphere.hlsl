cbuffer ViewProjMat	: register(b0)
{
    float4x4 ViewProj;
	float3 cameraPos;
	float3 eyePos;
	float2 padding;
}

cbuffer ObjectMat	: register(b1)
{
	float4x4 matFinal;
}


TextureCube dynamicCubeMap;

struct VOut
{
	float4 posH : SV_POSITION;
    float3 posL : POSITION;
    float3 norm : NORMALVO;
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

	output.posH = mul(mul(ViewProj, matFinal), float4(position, 1.0f));    // transform the vertex from 3D to 2D
    output.posL = position;
	output.norm = mul( transpose(matFinal), float4(normal, 1.0f)).xyz;

    return output;
}


float4 PShader(VOut input) : SV_TARGET
{   
	float3 incident = -eyePos;
	float3 reflectionVector = reflect(incident, input.norm);

	//return float4(1.0, 1.0, 0.0, 1.0);
	float4 color =  dynamicCubeMap.Sample(samTriLinearSam, input.posL);//float4(reflectionVector,1.0f);
    color.a = 1.0f;
    return color;
}
