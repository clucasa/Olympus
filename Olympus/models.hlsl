float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW);

cbuffer ConstantBuffer : register(b0)
{
    float4x4 matFinal;
	float3 cameraPos;
	float padding;
}

cbuffer worldBuffer	   : register(b1)
{
    float4x4 matWorld;
	float4x4 matWorldInvTrans;
}

struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Direction;
	float  SpecPower;
	float3 pad;
};


cbuffer DirectionalLight : register(b2)
{
	struct DirectionalLight dirLight[2];
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


Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);

struct VOut
{
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
	float3 TangentW   : TANGENT;
	float3 BiNormalW  : BINORM;
	float2 Tex        : TEXCOORD0;
	float3 CamPos     : CAMPOS;
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
	
	output.PosW		= mul(matWorld, input.Pos);
	output.NormalW  = normalize(mul((float3x3)matWorld, input.Normal));
	output.TangentW = normalize(mul((float3x3)matWorld, cross(input.Pos, input.Normal)));
	output.BiNormalW = normalize(mul((float3x3)matWorld, input.BiNormal));

	output.PosH		= mul( mul(matFinal, matWorld), input.Pos);
	
	output.Tex		= input.Tex;
	
	output.CamPos = cameraPos;

    return output;
}


float4 PShader(VOut input) : SV_TARGET
{
	float3 lightVec;
	float diffuseFactor;
	float specFactor;
	float3 v ;

	input.NormalW = normalize(input.NormalW);

	float3 toEye = cameraPos - input.PosW;
	float distToEye = length(toEye);
	toEye /= distToEye;

	
	

	float3 normalColor  = normalTexture.Sample( samLinear, input.Tex ).rgb;
	
	//return float4(normalColor.xyz, 1.0f);

	float3 bumpedNormalW = normalize(NormalSampleToWorldSpace(normalColor, input.NormalW, input.TangentW));

	
	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	for(int i = 0; i < 1; i++)
	{
		ambient	+= saturate(dirLight[i].Ambient);

		lightVec = -dirLight[i].Direction.xyz;

		diffuseFactor = dot(lightVec, bumpedNormalW);


		if(diffuseFactor > 0.0f)
		{
			diffuse += saturate(diffuseFactor * dirLight[i].Diffuse);
			
			v = reflect(-lightVec, bumpedNormalW);

			specFactor	 = pow(max(dot(v, toEye), 0.0f), dirLight[i].SpecPower);
			
			//spec    = saturate(specFactor * dirLight[i].Specular);
		}
	
	}

	float4 textureColor = float4(1.0f,1.0f,0.0f,1.0f);//float4(0.0f, 0.0f, 0.0f, 1.0f);
	textureColor = diffuseTexture.Sample( samLinear, input.Tex );
	//return float4(textureColor.rgb,1.0f);
	color = saturate(textureColor*(ambient + diffuse) + spec);
	//clip(color.a < 0.999999f ? -1:1 );

	color.a = 1.0;

    return color;
}

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f*normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N)*N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT,TBN);

	return bumpedNormalW;
}