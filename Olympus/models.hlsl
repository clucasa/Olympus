cbuffer ConstantBuffer
{
    float4x4 matFinal;
}

Texture2D diffuseTexture[3];
Texture2D normalTexture[3];

struct VOut
{
    float4 posH : SV_POSITION;
    float3 posL : POSITION;
	float2 Tex	: TEXCOORD;
	int  Texnum : TEXNUM;
};

struct Vin
{
	float4 Pos		: POSITION;
	float4 Normal	: NORMAL;
	float2 Tex		: TEXCOORD;
	int TexNum	    : TEXNUM;
	float4 Tangent	: TANGENT;
	float4 BiNormal	: BINORMAL;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

//VOut VShader(float3 position : POSITION, float3 normal : NORMAL, float3 tangent : TANGENT, float2 texcoord : TEXCOORD )
VOut VShader( Vin input )
{
    VOut output;

    output.posH = mul( matFinal, input.Pos );
    output.posL = input.Pos;
	output.Tex = input.Tex;
	output.Texnum = input.TexNum;

    return output;
}


float4 PShader(VOut input) : SV_TARGET
{
	//return 1.0;
	float4 color = 1.0;
	//if( input.Texnum == 0 )
		 color = diffuseTexture[0].Sample( samLinear, input.Tex );
	//else if( input.Texnum == 1 )
	//	color =  diffuseTexture[1].Sample( samLinear, input.Tex );
	//color =  diffuseTexture[2].Sample( samLinear, input.Tex );
	color.a = 1.0;
	return color;
	return 1.0;
}
