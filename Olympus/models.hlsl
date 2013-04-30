#include "LightHelper.hlsl"


cbuffer ConstantBuffer : register(b0)
{
    float4x4 ViewProj;
	float3 cameraPos;
	float padding;
}

cbuffer worldBuffer	   : register(b1)
{
    float4x4 matWorld;
	float4x4 matWorldInvTrans;
	Material material;
}

cbuffer DirectionalLight : register(b2)
{
	struct DirectionalLight dirLight[2];
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
	
	output.PosW		 = mul(matWorld, input.Pos);
	output.NormalW   = normalize(mul((float3x3)matWorldInvTrans, input.Normal));
	output.TangentW  = normalize(mul((float3x3)matWorld, input.Tangent));//cross(input.Pos, input.Normal)));
	output.BiNormalW = normalize(mul((float3x3)matWorld, input.BiNormal));

	output.PosH		 = mul( mul(ViewProj, matWorld), input.Pos);
	
	output.Tex		 = input.Tex;
	
	output.CamPos    = cameraPos;

    return output;
}


float4 PShader(VOut input) : SV_TARGET
{

	//return material.Specular;

	float3 lightVec;
	float diffuseFactor;
	float specFactor;
	float3 v ;
	float d;

	input.NormalW = normalize(input.NormalW);

	float3 toEye = cameraPos - input.PosW;
	float distToEye = length(toEye);
	toEye /= distToEye;

	
	

	float3 normalColor  = normalTexture.Sample( samLinear, input.Tex ).rgb;
	
	//return float4(normalColor.xyz, 1.0f);

	float3 bumpedNormalW = normalize(NormalSampleToWorldSpace(normalColor, input.NormalW, input.TangentW));

	
	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 totalAmbient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 totalDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 totalSpec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 dirAmbient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 dirDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 dirSpec    = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	float4 pAmbient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 pDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 pSpec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	for(int i = 0; i < 1; i++)
	{
		dirAmbient	+= saturate(material.Ambient * dirLight[i].Ambient);


		//lightVec = -dirLight[i].Direction.xyz;
		lightVec = -dirLight[i].Direction.xyz;
		lightVec = normalize(lightVec);
		diffuseFactor = dot(lightVec, bumpedNormalW);


		if(diffuseFactor > 0.0f)
		{
			dirDiffuse += saturate(diffuseFactor * material.Diffuse * dirLight[i].Diffuse);
			
			v = reflect(-lightVec, bumpedNormalW);

			specFactor	 = pow(max(dot(v, toEye), 0.0f), material.Specular.w);
			
			dirSpec    += saturate(specFactor * material.Specular * dirLight[i].Specular);
		}
	
	}

	totalAmbient += dirAmbient;
	totalDiffuse += dirDiffuse;
	totalSpec	 += dirSpec;

	for(int i = 0; i < 2; i++)
	{
		lightVec = pLight[i].Position - input.PosW;

		d = length(lightVec);

		if(d > pLight[i].Range)
			continue;

		lightVec /= d;

		pAmbient = material.Ambient * pLight[i].Ambient;

		diffuseFactor = dot(lightVec, bumpedNormalW);

		[flatten]
		if(diffuseFactor > 0.0f)
		{
			v = reflect(-lightVec, bumpedNormalW);

			specFactor = pow(max(dot(v, toEye), 0.0f), 5);

			pDiffuse = diffuseFactor * material.Diffuse * pLight[i].Diffuse;
			pSpec	= specFactor * material.Specular * pLight[i].Specular;
		}

		float att = 1.0f / dot(pLight[i].Att, float3(1.0f, d, d*d));

		pDiffuse *= att*(d / pLight[i].Range );
		pSpec    *= att*(d / pLight[i].Range );

		float softie = .55;

		if( d < softie*pLight[i].Range )
			pAmbient  *= 1/((d/pLight[i].Range+1)*(d/pLight[i].Range+1));
		if( d > softie*pLight[i].Range )
		{
			pAmbient *= 1/((softie*pLight[i].Range/pLight[i].Range+1)*(softie*pLight[i].Range/pLight[i].Range+1));
			pAmbient *= (pLight[i].Range-d)/(pLight[i].Range-softie*pLight[i].Range);
		}

		totalAmbient += pAmbient;
		totalDiffuse += pDiffuse;
		totalSpec	 += pSpec;
	}


	float4 textureColor = float4(1.0f,1.0f,0.0f,1.0f);//float4(0.0f, 0.0f, 0.0f, 1.0f);
	textureColor = diffuseTexture.Sample( samLinear, input.Tex );
	//return float4(textureColor.rgb,1.0f);
	color = saturate(textureColor*(totalAmbient + totalDiffuse) + totalSpec);
	//clip(color.a < 0.999999f ? -1:1 );

	color.a = 1.0;

    return color;
}