cbuffer ViewProjMat	: register(b0)
{
    float4x4 ViewProj;
	float3 cameraPos;
	float padding;
}

cbuffer ObjectMat	: register(b1)
{
	float4x4 matFinal;
	float4x4 matInvFinal;
}


TextureCube dynamicCubeMap;

struct VOut
{
	float4 posH : SV_POSITION;
    float3 posL : POSITION;
    float3 posW : POSITIONW;
    float3 norm : NORMALVO;
	float4 color : COLOR;
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
    output.posW = mul(matFinal, float4(position, 1.0f)).xyz;
	output.norm = mul((float3x3)matInvFinal, normalize(normal)); //normalize(mul((float3x3)matFinal, normal));
	output.color = matInvFinal[3];

    return output;
}


float4 PShader(VOut input) : SV_TARGET
{   
	//return float4(cameraPos, 1.0f);
	float3 incident = input.posW - cameraPos;
	float3 reflectionVector = reflect(incident, input.norm);
	//return float4((reflectionVector),1.0f);
	//return float4(padding, 1.0, 0.0, 1.0);
	float4 color =  dynamicCubeMap.Sample(samTriLinearSam, reflectionVector);//float4(reflectionVector,1.0f);
	//float4 color =  dynamicCubeMap.Sample(samTriLinearSam, input.posL);//float4(reflectionVector,1.0f);
    color.a = 1.0f;
    return color;
}
