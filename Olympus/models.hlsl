#include "LightHelper.hlsl"
#include "ConstBuffers.hlsl"

cbuffer SceneBuff : register(b0)
{
    SceneBuffer sceneBuff;
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


cbuffer ShadowProj  : register(b5)
{
    float4x4 lightViewProj;
    float3 lightPos;
    float PADdyCake;
}

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D shadowTexture : register(t6);

SamplerComparisonState ss : register(s5);

struct VOut
{
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float3 BiNormalW  : BINORM;
    float2 Tex        : TEXCOORD0;
    float3 CamPos     : CAMPOS;
    float4 lpos    : TEXCOORD2;
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

    output.PosH		 = mul( mul(sceneBuff.ViewProj, matWorld), input.Pos);

    output.Tex		 = input.Tex;

    output.CamPos    = sceneBuff.cameraPos;

    output.lpos = mul( matWorld, float4(input.Pos.xyz, 1.0) );
    output.lpos = mul( lightViewProj, float4(output.lpos.xyz, 1.0) );

    return output;
}

float3 offset_lookup(Texture2D map, float4 loc, float2 offset)
{
//return tex2Dproj(map, float4(loc.xy + offset * float2(1/4096,1/4096) * loc.w, loc.z, loc.w));
//return map.SampleCmpLevelZero( ss, (loc.xy + offset * float2(1/4096,1/4096)) * loc.w ,  loc.z, loc.w);
//return map.SampleCmpLevelZero( ss, float4(loc.xy + offset * float2(1/4096,1/4096) * loc.w ,  loc.z, loc.w));
//return map.SampleCmp(ss, loc.xy + offset * (1/4096) * loc.w, loc.z).r;
return map.SampleCmpLevelZero(ss, loc.xy + offset, loc.z).r;
}
float shadowVal(VOut input)
{
    //return PADdyCake;
    //return theShade.Sample( samLinear, input.texcoord );

    //  return 1.0;
    //  return float4( PADdyCake, PADdyCake, PADdyCake, 1.0 );
    //  return float4( lightPos.x, lightPos.y, lightPos.z, 1.0 );
    input.lpos.xy /= input.lpos.w;

    if( input.lpos.x < -1.0f || input.lpos.x > 1.0f ||
        input.lpos.y < -1.0f || input.lpos.y > 1.0f ||
        input.lpos.z < 0.0f  || input.lpos.z > 1.0f ) return 1.0;

    //return 0.0;

    input.lpos.x = input.lpos.x/2 + 0.5;
    input.lpos.y = input.lpos.y/-2 + 0.5;

    input.lpos.z -= .0015;


    //float samp = 1000 * 1/4096;
    float samp = .0005;

    float3 shadowCoeff = (
        offset_lookup(shadowTexture, input.lpos, float2(0, 0))+
        offset_lookup(shadowTexture, input.lpos, float2(0, -samp)) +
        offset_lookup(shadowTexture, input.lpos, float2(0, samp)) +
        offset_lookup(shadowTexture, input.lpos, float2(-samp, 0)) +
        offset_lookup(shadowTexture, input.lpos, float2(-samp, -samp)) +
        offset_lookup(shadowTexture, input.lpos, float2(-samp, samp)) +
        offset_lookup(shadowTexture, input.lpos, float2( samp, 0)) +
        offset_lookup(shadowTexture, input.lpos, float2( samp, -samp)) +
        offset_lookup(shadowTexture, input.lpos, float2( samp, samp)) 
        ) * 1.f/9.f;

    return shadowCoeff.r;

    float shad = shadowTexture.Sample( samLinear, input.lpos.xy ).r;
//	return shad;
    if ( shad < input.lpos.z) return 0.0;

    return 1.0;
}


float4 PShader(VOut input) : SV_TARGET
{

    float4 textureColor = float4(1.0f,1.0f,0.0f,1.0f);//float4(0.0f, 0.0f, 0.0f, 1.0f);

    if(sceneBuff.textures == 1.0f)
        textureColor = diffuseTexture.Sample( samLinear, input.Tex );
    else
        textureColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

    
    if( material.AlphaKillOn )
        clip( textureColor.a - .6f );


    //return material.Specular;
    
    if(material.AlphaKillOn)
    {
        clip(diffuseTexture.Sample( samLinear, input.Tex ).a < 0.5f ? -1:1 );
    }
    float3 lightVec;
    float diffuseFactor;
    float specFactor;
    float3 v ;
    float d;

    input.NormalW = normalize(input.NormalW);

    float3 toEye = input.CamPos - input.PosW;
    float distToEye = length(toEye);
    toEye /= distToEye;

    float3 bumpedNormalW;

    if(sceneBuff.normalMap == 1.0f)
    {
        float3 normalColor  = normalTexture.Sample( samLinear, input.Tex ).rgb;
        bumpedNormalW = normalize(NormalSampleToWorldSpace(normalColor, input.NormalW, input.TangentW));
    }
    else
        bumpedNormalW = input.NormalW;

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

    float shadow = 1.0f;
    
    if(sceneBuff.shadowsOn == 1.0f)
        shadow = shadowVal(input);

    for(int i = 0; i < 1; i++)
    {
        if(sceneBuff.ambientOn == 1.0f)
        {
            dirAmbient	+= saturate(material.Ambient * dirLight[i].Ambient);
        }

        //lightVec = -dirLight[i].Direction.xyz;
        lightVec = -dirLight[i].Direction.xyz;
        lightVec = normalize(lightVec);
        diffuseFactor = dot(lightVec, bumpedNormalW);


        if(diffuseFactor > 0.0f)
        {
            if(sceneBuff.diffuseOn == 1.0f)
            {
                dirDiffuse += saturate(diffuseFactor * material.Diffuse * dirLight[i].Diffuse);
            }
            v = reflect(-lightVec, bumpedNormalW);

            specFactor	 = pow(max(dot(v, toEye), 0.0f), material.Specular.w);

            if(sceneBuff.specularOn == 1.0f)
            {
                dirSpec    += specFactor * material.Specular * dirLight[i].Specular;
            }
        }

    }

    if(sceneBuff.dirLightOn == 1.0f)
    {
        if(sceneBuff.ambientOn == 1.0f)
            totalAmbient += dirAmbient;

        if(sceneBuff.diffuseOn == 1.0f)
            totalDiffuse += dirDiffuse*shadow;

        if(sceneBuff.specularOn == 1.0f)
            totalSpec	 += dirSpec*shadow;
    }

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

        float softie = .75;

        if( d < softie*pLight[i].Range )
        {
            pAmbient  *= 1.0f/((d/pLight[i].Range+1.0f)*(d/pLight[i].Range+1.0f));
            pDiffuse  *= 1.0f/((d/pLight[i].Range+1.0f)*(d/pLight[i].Range+1.0f));
        }
        if( d > softie*pLight[i].Range )
        {
            pAmbient *= 1.0f/((softie*pLight[i].Range/pLight[i].Range+1.0f)*(softie*pLight[i].Range/pLight[i].Range+1.0f));
            pAmbient *= (pLight[i].Range-d)/(pLight[i].Range-softie*pLight[i].Range);
            pDiffuse *= 1.0f/((softie*pLight[i].Range/pLight[i].Range+1.0f)*(softie*pLight[i].Range/pLight[i].Range+1.0f));
            pDiffuse *= (pLight[i].Range-d)/(pLight[i].Range-softie*pLight[i].Range);
        }

        if(sceneBuff.pLightOn == 1.0f)
        {
            if(sceneBuff.ambientOn == 1.0f)
                totalAmbient += (4.0f * pAmbient);
            if(sceneBuff.diffuseOn == 1.0f)
                totalDiffuse += (4.0f * pDiffuse);
            //if(sceneBuff.specularOn == 1.0f)
            //totalSpec	 += pSpec;
        }
    }


    

    color = textureColor*(totalAmbient + totalDiffuse) + totalSpec;
    //clip(color.a < 0.999999f ? -1:1 );

    color.a = 1.0;

    
    return color;
}

void PSAplhaShadow(VOut input)
{
    float4 textureColor = float4(1.0f,1.0f,0.0f,1.0f);//float4(0.0f, 0.0f, 0.0f, 1.0f);

    if(sceneBuff.textures == 1.0f)
        textureColor = diffuseTexture.Sample( samLinear, input.Tex );
    else
        textureColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

    if( material.AlphaKillOn )
        clip( textureColor.a - .6f );
}