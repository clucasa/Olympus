cbuffer ViewProjMat	: register(c0)
{
    float4x4 ViewProj;
}

cbuffer ObjectMat	: register(c1)
{
	float3	eyePos;
	float buffer;
}

//cbuffer ConstantBuffer
//{
//  float4x4 final;
//	float3	eyePos;
//	float buffer;
//}

struct VOut
{
	float3 Position : POSITION;
};

struct GOut
{
	float4 Position : SV_POSITION;
	uint primID		: SV_PrimitiveID;
    float2 texcoord : TEX_COORDS;
};

Texture2D theTexture;

SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

VOut VShader(float3 position : POSITION)
{
	VOut output;
	output.Position = position;
	return output;
}

[maxvertexcount(4)]
void GShader(point VOut input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GOut> triangleStream)
{
	// Compute local coordinate system to billboard to face eye
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = eyePos - input[0].Position;
	//look.y = 0.0f;
	look = normalize(look);
	float3 left = cross(up, look);
        //up = cross(look, left);

	// Compute triangle strip vertices of the quad
	float halfWidth = 0.075;
	float halfHeight = 0.075;

	float4 bottomLeft	= float4(input[0].Position + halfWidth * left - halfHeight * up, 1.0f);
	float4 topLeft		= float4(input[0].Position + halfWidth * left + halfHeight * up, 1.0f);
	float4 bottomRight	= float4(input[0].Position - halfWidth * left - halfHeight * up, 1.0f);
	float4 topRight		= float4(input[0].Position - halfWidth * left + halfHeight * up, 1.0f);

	float4 verts[4] = { bottomLeft, topLeft, bottomRight, topRight };
    float2 texc[4]  = { float2(0.0f,0.0f), float2(0.0f,1.0f), float2(1.0f, 0.0f), float2(1.0f, 1.0f) }; 

	GOut output;
	[unroll]
	for(int i = 0; i < 4; i++)
	{
		output.Position = mul(ViewProj, verts[i]);
		output.primID = primID;		
        output.texcoord = texc[i];
		triangleStream.Append(output);
	}
}

float4 PShader(GOut input) : SV_TARGET
{
	float4 color = float4(0.0f, 0.502f, 0.753f, 1.0f);//theTexture.Gather(samTriLinearSam, input.texcoord, int2(0,0));
    color.a = theTexture.GatherAlpha(samTriLinearSam, input.texcoord, int2(0,0), int2(0,0), int2(0,0), int2(0,0));
	
	//clip(color.a < 0.1f ? -1:1);
	//color.a = 1.0f;
    return color;
}