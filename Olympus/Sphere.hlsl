#include "LightHelper.hlsl"

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
	Material material;
}


TextureCube dynamicCubeMap;

struct VOut
{
	float4 posH : SV_POSITION;
    float3 posL : POSITION;
    float3 PosW : POSITIONW;
    float3 NormalW : NORMALVO;
	float4 color : COLOR;
};

SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

cbuffer DirectionalLight : register(b2)
{
	struct DirectionalLight dirLight[2];
};

cbuffer PointLight : register(b3)
{
	struct PointLight pLight[2];
};


VOut VShader(float3 position : POSITION, float3 normal : NORMAL, float3 tangent : TANGENT, float2 texcoord : TEXCOORD )
{
    VOut output;

	output.posH = mul(mul(ViewProj, matFinal), float4(position, 1.0f));    // transform the vertex from 3D to 2D
    output.posL = position;
    output.PosW = mul(matFinal, float4(position, 1.0f)).xyz;
	output.NormalW = mul((float3x3)matInvFinal, normalize(normal)); //normalize(mul((float3x3)matFinal, normal));
	output.color = matInvFinal[3];

    return output;
}


float4 PShader(VOut input) : SV_TARGET
{   

	float3 incident = input.PosW - cameraPos;
	float3 reflectionVector = reflect(incident, input.NormalW);

	float3 lightVec;
	float diffuseFactor;
	float specFactor;
	float3 v ;
	float d;

	input.NormalW = normalize(input.NormalW);

	float3 toEye = cameraPos - input.PosW;
	float distToEye = length(toEye);
	toEye /= distToEye;

	
	//float3 normalColor  = normalTexture.Sample( samLinear, input.Tex ).rgb;

	float3 bumpedNormalW = input.NormalW;//normalize(NormalSampleToWorldSpace(normalColor, input.NormalW, input.TangentW));

	
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
		dirAmbient	+= saturate(dirLight[i].Ambient);


		//lightVec = -dirLight[i].Direction.xyz;
		lightVec = -dirLight[i].Direction.xyz;
		lightVec = normalize(lightVec);
		diffuseFactor = dot(lightVec, bumpedNormalW);


		if(diffuseFactor > 0.0f)
		{
			dirDiffuse += saturate(diffuseFactor * dirLight[i].Diffuse);
			
			v = reflect(-lightVec, bumpedNormalW);

			specFactor	 = pow(max(dot(v, toEye), 0.0f), dirLight[i].Specular.w);
			
			dirSpec    += saturate(specFactor * dirLight[i].Specular);
		}
	
	}
    //return float4(dirSpec.xyz,1.0f);
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

		pAmbient = pLight[i].Ambient;

		diffuseFactor = dot(lightVec, bumpedNormalW);

		[flatten]
		if(diffuseFactor > 0.0f)
		{
			v = reflect(-lightVec, bumpedNormalW);

			specFactor = pow(max(dot(v, toEye), 0.0f), 5);

			pDiffuse = diffuseFactor * pLight[i].Diffuse;
			pSpec	= specFactor * pLight[i].Specular;
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



	color =  dynamicCubeMap.Sample(samTriLinearSam, reflectionVector);

	color = color*(totalAmbient + totalDiffuse) + totalSpec;

    color.a = 1.0f;
    return color;
}
