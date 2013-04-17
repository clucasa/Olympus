Texture2D gSpriteTex;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosNdc : POSITION;
	float2 Tex    : TEXCOORD;
	float4 Color  : COLOR;
};

struct VertexOut
{
	float4 PosNdc : SV_POSITION;
    float2 Tex    : TEXCOORD;
	float4 Color  : COLOR;
};
 
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.PosNdc = float4(vin.PosNdc, 1.0f);
	vout.Tex    = vin.Tex;
	vout.Color  = vin.Color;
	
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	//return 1.0;
	return pin.Color*gSpriteTex.Sample(samLinear, pin.Tex);   
}