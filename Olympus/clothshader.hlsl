#include "LightHelper.hlsl"
#include "ConstBuffers.hlsl"

cbuffer SceneBuff : register(b0)
{
	SceneBuffer sceneBuff;
};

cbuffer worldBuffer	   : register(b1)
{
	float4x4 matWorld;
	float4x4 matWorldInvTrans;
	Material material;
};

cbuffer DirectionalLight : register(b2)
{
	struct DirectionalLight dirLight[2];
};

cbuffer PointLight : register(b3)
{
	struct PointLight pLight[2];
};

struct Vin
{
	//float2 Tex		: TEXCOORD;
	float3 Pos		: POSITION;
	float3 Normal	: NORMAL;
};

struct VOut
{
	float4 PosH       : SV_POSITION;
	//float3 PosW       : POSITION;
	float3 NormalW    : NORMALW;
	//float3 TangentW   : TANGENT;
	//float3 BiNormalW  : BINORM;
	//float2 Tex        : TEXCOORD0;
	//float3 CamPos     : CAMPOS;
	//float4 lpos       : TEXCOORD2;
};

Texture2D theTexture : register(t0);

VOut VShader( Vin input )
{
	VOut output;
	//output.PosW		 = mul(matWorld, input.Pos);
	output.NormalW   = normalize(input.Normal);//mul((float3x3)matWorldInvTrans, 
	//output.TangentW  = normalize(mul((float3x3)matWorld, input.Tangent));//cross(input.Pos, input.Normal)));
	//output.BiNormalW = normalize(mul((float3x3)matWorld, input.BiNormal));

	output.PosH		 = mul( sceneBuff.ViewProj, float4(input.Pos,1.0f));

	//output.Tex		 = input.Tex;

	//output.CamPos    = sceneBuff.cameraPos;

	//output.lpos = mul( matWorld, float4(input.Pos.xyz, 1.0) );
	//output.lpos = mul( lightViewProj, float4(output.lpos.xyz, 1.0) );

	return output;
}


float4 PShader(VOut input) : SV_TARGET
{	
	return float4(input.NormalW,1.0);
	//float4 textureColor;
 //   float3 lightDir;
 //   float lightIntensity;
 //   float3 reflection;
 //   float4 specular;
	//float3 diffuse;
	//float3 ambient;
	//float3 halfway;

	//float4 totalAmbient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//float4 totalDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//float4 color = theTexture.Sample( ss, input.texcoord );///*float4(0.3f, 0.502f, 0.753f, 1.0f);*/theTexture.Gather(samTriLinearSam, input.texcoord, int2(0,0));
	//

	//for(int i = 0; i < 2; i++)
	//{
	//	ambient = dirLight[i].Ambient.xyz;

	//	diffuse = dirLight[i].Diffuse.xyz;// * saturate(dot(input.lookAt.xyz, -Direction.xyz));

	//	totalAmbient.xyz += ambient;
	//	totalDiffuse.xyz += diffuse;
	//}	
	//


	//float4 pAmbient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//float4 pDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//
	///*for(int i = 0; i < 2; i++)
	//{
	//	float3 lightVec = pLight[i].Position - input.posW;

	//	float d = length(lightVec);

	//	if(d > pLight[i].Range)
	//		continue;

	//	lightVec /= d; 

	//	pAmbient = pLight[i].Ambient;

	//	float diffuseFactor = dot(-lightVec, input.lookAt);


	//	pDiffuse = pLight[i].Diffuse;

	//	float att = 1.0f / dot(pLight[i].Att, float3(1.0f, d, d*d));

	//	pDiffuse *= att*(d / pLight[i].Range );

	//	float softie = .75;

	//	if( d < softie*pLight[i].Range )
	//		pAmbient  *= 1/((d/pLight[i].Range+1)*(d/pLight[i].Range+1));
	//	if( d > softie*pLight[i].Range )
	//	{
	//		pAmbient *= 1/((softie*pLight[i].Range/pLight[i].Range+1)*(softie*pLight[i].Range/pLight[i].Range+1));
	//		pAmbient *= (pLight[i].Range-d)/(pLight[0].Range-softie*pLight[0].Range);
	//	}


	//	totalAmbient.xyz +=  pAmbient.xyz;
	//	totalDiffuse.xyz +=  pDiffuse.xyz;
	//}*/



	//color *= totalDiffuse + totalAmbient;///(5.0f-numLightsHit);
	//color = saturate(color);
	//color.a = theTexture.GatherAlpha(ss, input.texcoord, int2(0,0), int2(0,0), int2(0,0), int2(0,0));

	//clip(color.a < 0.999999f ? -1:1 );

	//return color;
}