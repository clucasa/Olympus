cbuffer ConstantBuffer
{
    float4x4 final;
	float3	eyePos;
	float buffer;
}

struct VOut
{
	float3 Position : POSITION;
};

struct GOut
{
	float4 Position : SV_POSITION;
	uint primID		: SV_PrimitiveID;
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
	look.y = 0.0f;
	look = normalize(look);
	float3 left = cross(up, look);

	// Compute triangle strip vertices of the quad
	float halfWidth = 0.05;
	float halfHeight = 0.05;

	float4 bottomLeft	= float4(input[0].Position + halfWidth * left - halfHeight * up, 1.0f);
	float4 topLeft		= float4(input[0].Position + halfWidth * left + halfHeight * up, 1.0f);
	float4 bottomRight	= float4(input[0].Position - halfWidth * left - halfHeight * up, 1.0f);
	float4 topRight		= float4(input[0].Position - halfWidth * left + halfHeight * up, 1.0f);

	float4 verts[4] = { bottomLeft, topLeft, bottomRight, topRight };

	GOut output;
	[unroll]
	for(int i = 0; i < 4; i++)
	{
		output.Position = mul(final, verts[i]);
		output.primID = primID;		
		triangleStream.Append(output);
	}
}

float4 PShader() : SV_TARGET
{
	return float4(1.0f, 0.5f, 0.2f, 1.0f);
}