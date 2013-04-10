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
	float halfWidth = 0.25;
	float halfHeight = 0.25;

	/*float4 v[4];
	v[0] = float4(-halfWidth, -halfHeight, 0.0f, 1.0f);
	v[1] = float4(+halfWidth, -halfHeight, 0.0f, 1.0f);
	v[2] = float4(-halfWidth, +halfHeight, 0.0f, 1.0f);
	v[3] = float4(+halfWidth, +halfHeight, 0.0f, 1.0f);

	float4x4 W;
	W[0] = float4(right,          0.0f);
	W[1] = float4(up,             0.0f);
	W[2] = float4(look,           0.0f);
	W[3] = float4(input[0].Position, 1.0f);

	float4x4 WVP = mul(W,final);

	GOut output;
    [unroll]
    for(int i = 0; i < 4; ++i)
    {
            output.Position = mul(v[i], final);
			output.primID = primID;	
            triangleStream.Append(output);
    }*/

	float4 bottomLeft	= float4(input[0].Position + halfWidth * left - halfHeight * up, 1.0f);
	float4 topLeft		= float4(input[0].Position + halfWidth * left + halfHeight * up, 1.0f);
	float4 bottomRight	= float4(input[0].Position - halfWidth * left - halfHeight * up, 1.0f);
	float4 topRight		= float4(input[0].Position - halfWidth * left + halfHeight * up, 1.0f);

	float4 verts[4] = { bottomLeft, topLeft, bottomRight, topRight };

	GOut output;
	[unroll]
	for(int i = 0; i < 4; i++)
	{
		output.Position = mul(verts[i], final);
		output.primID = primID;		
		triangleStream.Append(output);
	}
}

float4 PShader() : SV_TARGET
{
	return float4(0.9, 0.9, 1.0, 1.0);
}