cbuffer ViewProjMat	: register(c0)
{
    float4x4 ViewProj;
}

cbuffer ObjectMat	: register(c1)
{
	float3	eyePos;
	float buffer;
}

cbuffer DirectionalLight : register(b2)
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Direction;
	float  SpecPower;
	float3 pad;
};

struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	// Packed into 4D vector: (Position, Range)
	float3 Position;
	float Range;

	// Packed into 4D vector: (A0, A1, A2, Pad)
	float3 Att;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

cbuffer PointLight : register(b3)
{
	struct PointLight pLight[2];
};

struct VOut
{
	float3 Position : POSITION;
};

struct GOut
{
	float4 Position : SV_POSITION;
	uint primID		: SV_PrimitiveID;
    float2 texcoord : TEX_COORDS;
	float3 lookAt	: LOOKAT;
	float3 posW		: WORLDPOSITION;
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
    //up = cross(left, look);

	// Compute triangle strip vertices of the quad
	float halfWidth = 0.15;
	float halfHeight = 0.15;

	float4 bottomLeft	= float4(input[0].Position + halfWidth * left - halfHeight * up, 1.0f);
	float4 topLeft		= float4(input[0].Position + halfWidth * left + halfHeight * up, 1.0f);
	float4 bottomRight	= float4(input[0].Position - halfWidth * left - halfHeight * up, 1.0f);
	float4 topRight		= float4(input[0].Position - halfWidth * left + halfHeight * up, 1.0f);

	float4 verts[4] = { bottomLeft, topLeft, bottomRight, topRight };
    float2 texc[4]  = { float2(1.0f,1.0f), float2(1.0f,0.0f), float2(0.0f, 1.0f), float2(0.0f, 0.0f) }; 

	GOut output;
	[unroll]
	for(int i = 0; i < 4; i++)
	{
		output.Position = mul(ViewProj, verts[i]);
		output.posW		= verts[i];
		output.primID = primID;		
        output.texcoord = texc[i];
		output.lookAt = look;
		triangleStream.Append(output);
	}

	
}

float4 PShader(GOut input) : SV_TARGET
{	
	float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float3 reflection;
    float4 specular;
	float3 diffuse;
	float3 ambient;
	float3 halfway;


	float4 color = theTexture.Sample( samTriLinearSam, input.texcoord );///*float4(0.3f, 0.502f, 0.753f, 1.0f);*/theTexture.Gather(samTriLinearSam, input.texcoord, int2(0,0));
	

	ambient = Ambient;

	diffuse = Diffuse;// * saturate(dot(input.lookAt.xyz, -Direction.xyz));
	
	//halfway = normalize(-Direction.xyz + input.lookAt);

	//specular.xyz = pow(saturate(dot(input.lookAt.xyz, halfway)), SpecPower*3) * Specular/2;

	


	float4 pAmbient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 pDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 pSpec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = pLight[0].Position - input.posW;
	float d = length(lightVec);

	if(d > pLight[0].Range)
	{
		color.xyz = saturate(diffuse.xyz) * color.xyz;
		color.a = theTexture.GatherAlpha(samTriLinearSam, input.texcoord, int2(0,0), int2(0,0), int2(0,0), int2(0,0));
		//return 1;
		return color;
	}

	lightVec /= d; 

	pAmbient = pLight[0].Ambient;

	float diffuseFactor = dot(-lightVec, input.lookAt);

	//if( diffuseFactor > 0.0f )
	//{
		//float3 v         = reflect(-lightVec, normal);
		//float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		pDiffuse = pLight[0].Diffuse;
		//spec    = specFactor * mat.Specular * L.Specular;
	//}

	float att = 1.0f / dot(pLight[0].Att, float3(1.0f, d, d*d));

	pDiffuse *= att*(d / pLight[0].Range );

	float softie = .75;

	if( d < softie*pLight[0].Range )
		pAmbient  *= 1/((d/pLight[0].Range+1)*(d/pLight[0].Range+1));
	if( d > softie*pLight[0].Range )
	{
		pAmbient *= 1/((softie*pLight[0].Range/pLight[0].Range+1)*(softie*pLight[0].Range/pLight[0].Range+1));
		pAmbient *= (pLight[0].Range-d)/(pLight[0].Range-softie*pLight[0].Range);
	}

	float4 totalAmbient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 totalDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//float4 totalSpec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//totalAmbient.xyz +=  ambient.xyz;
	totalAmbient.xyz +=  pAmbient.xyz;

	//totalDiffuse.xyz +=  diffuse.xyz;
	totalDiffuse.xyz +=  pDiffuse.xyz;

	//totalSpec.xyz	 +=  specular.xyz;
	//totalSpec.xyz	 +=  pSpec.xyz;


	color = saturate(totalAmbient + totalDiffuse);
	
	color.a = theTexture.GatherAlpha(samTriLinearSam, input.texcoord, int2(0,0), int2(0,0), int2(0,0), int2(0,0));

	clip(color.a < 0.9999f ? -1:1 );

	return color;

}