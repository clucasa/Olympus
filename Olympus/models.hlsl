#include "LightHelper.hlsl"
#include "ConstBuffers.hlsl"

#define NUMDIRLIGHTS 1
#define NUMPOINTLIGHTS 2

#define NUMSHADOWS 15
#define NUMDIRSHADOWS 3		// This is going to hold our cascade shadow maps for now
#define NUMPOINTSHADOWS 12


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
    struct DirectionalLight dirLight[10];
};

cbuffer PointLight : register(b3)
{
    struct PointLight pLight[21];
};


cbuffer Shadows  : register(b5)
{
    struct ShadowProj shadowProj[NUMSHADOWS];
}

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D shadowTexture[NUMSHADOWS] : register(t6);

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
    float4 lpos[NUMSHADOWS]    : TEXCOORD2;
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
    output.NormalW   = normalize(mul((float3x3)matWorld, input.Normal));
    output.TangentW  = normalize(mul((float3x3)matWorld, input.Tangent));//cross(input.Pos, input.Normal)));
    output.BiNormalW = normalize(mul((float3x3)matWorld, input.BiNormal));

    output.PosH		 = mul( mul(sceneBuff.ViewProj, matWorld), input.Pos);

    output.Tex		 = input.Tex;

    output.CamPos    = sceneBuff.cameraPos;

    for( int i = 0; i < NUMSHADOWS; i++ )
    {
        output.lpos[i] = mul( matWorld, float4(input.Pos.xyz, 1.0) );
        output.lpos[i] = mul( shadowProj[i].lightViewProj, float4(output.lpos[i].xyz, 1.0) );
    }

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
float shadowVal(VOut input, int i)
{
    //	input.lpos[i].xyz /= input.lpos[i].w;

    if( input.lpos[i].x < -1.0f || input.lpos[i].x > 1.0f ||
        input.lpos[i].y < -1.0f || input.lpos[i].y > 1.0f ||
        input.lpos[i].z < 0.0f  || input.lpos[i].z > 1.0f ) return  1.0;

    //return 0.0;

    input.lpos[i].x = input.lpos[i].x/2 + 0.5;
    input.lpos[i].y = input.lpos[i].y/-2 + 0.5;

    if( i == 0 )
        input.lpos[i].z -= .0005;
    if( i == 1 )
        input.lpos[i].z -= .003;
    if( i == 2 )
        input.lpos[i].z -= .003;

    //float samp = 1000 * 1/4096;
    float samp = .0005;

    float3 shadowCoeff = (
        offset_lookup(shadowTexture[i], input.lpos[i], float2(0, 0))+
        offset_lookup(shadowTexture[i], input.lpos[i], float2(0, -samp)) +
        offset_lookup(shadowTexture[i], input.lpos[i], float2(0, samp)) +
        offset_lookup(shadowTexture[i], input.lpos[i], float2(-samp, 0)) +
        offset_lookup(shadowTexture[i], input.lpos[i], float2(-samp, -samp)) +
        offset_lookup(shadowTexture[i], input.lpos[i], float2(-samp, samp)) +
        offset_lookup(shadowTexture[i], input.lpos[i], float2( samp, 0)) +
        offset_lookup(shadowTexture[i], input.lpos[i], float2( samp, -samp)) +
        offset_lookup(shadowTexture[i], input.lpos[i], float2( samp, samp)) 
        ) * 1.f/9.f;

    return shadowCoeff.r;
}


float shadowValPoint(VOut input, int start)
{
    int test;
    for( int i = start; i < (start+6); i++ )
    {
        input.lpos[i].xyz /= input.lpos[i].w;

        if( input.lpos[i].x < -1.0f || input.lpos[i].x > 1.0f ||
            input.lpos[i].y < -1.0f || input.lpos[i].y > 1.0f ||
            input.lpos[i].z < 0.0f  || input.lpos[i].z > 1.0f ) test = 1;
        else
        {
            //if( i == start )
            //return 0.0;
            input.lpos[i].x = input.lpos[i].x/2 + 0.5;
            input.lpos[i].y = input.lpos[i].y/-2 + 0.5;

            input.lpos[i].z -= .002;

            float samp = .001;

            float3 shadowCoeff = (
                offset_lookup(shadowTexture[i], input.lpos[i], float2(0, 0))+
                offset_lookup(shadowTexture[i], input.lpos[i], float2(0, -samp)) +
                offset_lookup(shadowTexture[i], input.lpos[i], float2(0, samp)) +
                offset_lookup(shadowTexture[i], input.lpos[i], float2(-samp, 0)) +
                offset_lookup(shadowTexture[i], input.lpos[i], float2(-samp, -samp)) +
                offset_lookup(shadowTexture[i], input.lpos[i], float2(-samp, samp)) +
                offset_lookup(shadowTexture[i], input.lpos[i], float2( samp, 0)) +
                offset_lookup(shadowTexture[i], input.lpos[i], float2( samp, -samp)) +
                offset_lookup(shadowTexture[i], input.lpos[i], float2( samp, samp)) 
                ) * 1.f/9.f;

            return shadowCoeff.r;
        }
    }
    return 1.0;
}


float4 PShader(VOut input) : SV_TARGET
{

    //return float4(pLight[2].Diffuse.xyz, 1.0f);

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
    //return float4(1.0f, diffuseFactor, 0.0f, 1.0f);
   // if(sceneBuff.shadowsOn == 1.0f)
       // shadow = shadowVal(input);

    for(int i = 0; i < NUMDIRLIGHTS; i++)
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
            float shadow = 1.0;
            //if( i < NUMDIRSHADOWS )
            //	shadow = shadowVal(input, i);
            if(sceneBuff.shadowsOn == 1.0f)

                for( int k = 0; k < NUMDIRSHADOWS; k++ )
                {
                    input.lpos[k].xyz /= input.lpos[k].w;

                    if( k == 0 )
                    {
                        if( input.lpos[k].x < -1.0f || input.lpos[k].x > 1.0f ||
                            input.lpos[k].y < -1.0f || input.lpos[k].y > 1.0f ||
                            input.lpos[k].z < 0.0f  || input.lpos[k].z > 1.0f ) shadow = 1.0;

                        else
                        {
                            if(sceneBuff.cascadeOn == 1.0f)
                            {
                                if( k == 0 )
                                    return float4( 1.0, 0.0, 0.0, 1.0 );
                                else if ( k == 1 )
                                    return float4( 0.0, 1.0, 0.0, 1.0 );
                                else if ( k == 2 )
                                    return float4( 0.0, 0.0, 1.0, 1.0 );
                            }

                            shadow = shadowVal( input, k );
                            if( shadow < 1.0 )
                            {

                                //if( k == 0 )
                                //	return float4( 1.0, 0.0, 0.0, 1.0 );
                                //else if ( k == 1 )
                                //	return float4( 0.0, 1.0, 0.0, 1.0 );
                                //else if ( k == 2 )
                                //	return float4( 0.0, 0.0, 1.0, 1.0 );
                                break;
                            }
                        }
                    }
                    else
                    {
                        if( input.lpos[k-1].x < -1.0f || input.lpos[k-1].x > 1.0f ||
                            input.lpos[k-1].y < -1.0f || input.lpos[k-1].y > 1.0f ||
                            input.lpos[k-1].z < 0.0f  || input.lpos[k-1].z > 1.0f )
                        {
                            if( input.lpos[k].x < -1.0f || input.lpos[k].x > 1.0f ||
                                input.lpos[k].y < -1.0f || input.lpos[k].y > 1.0f ||
                                input.lpos[k].z < 0.0f  || input.lpos[k].z > 1.0f ) shadow = 1.0;
                            else
                            {
                                if(sceneBuff.cascadeOn == 1.0f)
                                {
                                    if( k == 0 )
                                        return float4( 1.0, 0.0, 0.0, 1.0 );
                                    else if ( k == 1 )
                                        return float4( 0.0, 1.0, 0.0, 1.0 );
                                    else if ( k == 2 )
                                        return float4( 0.0, 0.0, 1.0, 1.0 );
                                }

                                shadow = shadowVal( input, k );
                                if( shadow < 1.0 )
                                {
                                    //if( k == 0 )
                                    //	return float4( 1.0, 0.0, 0.0, 1.0 );
                                    //else if ( k == 1 )
                                    //	return float4( 0.0, 1.0, 0.0, 1.0 );
                                    //else if ( k == 2 )
                                    //	return float4( 0.0, 0.0, 1.0, 1.0 );
                                    break;
                                }
                            }
                        }
                    }
                }


            if(sceneBuff.diffuseOn == 1.0f)
            {
                dirDiffuse += saturate(diffuseFactor * material.Diffuse * dirLight[i].Diffuse) * shadow;
            }
            v = reflect(-lightVec, bumpedNormalW);

            specFactor	 = pow(max(dot(v, toEye), 0.0f), material.Specular.w);

            if(sceneBuff.specularOn == 1.0f)
            {
                dirSpec    += specFactor * material.Specular * dirLight[i].Specular * shadow;
            }
        }

    }

    if(sceneBuff.dirLightOn == 1.0f)
    {
        if(sceneBuff.ambientOn == 1.0f)
            totalAmbient += dirAmbient;

        if(sceneBuff.diffuseOn == 1.0f)
            totalDiffuse += dirDiffuse;

        if(sceneBuff.specularOn == 1.0f)
            totalSpec	 += dirSpec;
    }

    [unroll]
    for(int i = 0; i < pLight[0].pad; i++)
    {
        lightVec = pLight[i].Position - input.PosW;

        d = length(lightVec);

        lightVec /= d;

        pAmbient = material.Ambient * pLight[i].Ambient;

        diffuseFactor = dot(lightVec, bumpedNormalW);

        [flatten]
        if(diffuseFactor > 0.0f)
        {
            v = reflect(-lightVec, bumpedNormalW);

            specFactor = pow(max(dot(v, toEye), 0.0f), material.Specular.w);

            pDiffuse = diffuseFactor * material.Diffuse * pLight[i].Diffuse;
            pSpec	= specFactor * material.Specular * pLight[i].Specular;
        }

        if(d > pLight[i].Range)
            continue;

        float att = 1.0f / dot(pLight[i].Att, float3(1.0f, d, d*d));

        pDiffuse *= att*(d / pLight[i].Range );
        pSpec    *= att*(d / pLight[i].Range );

        float softie = .75;

        if( d < softie*pLight[i].Range )
        {
            pAmbient  *= 1.0f/((d/pLight[i].Range+1.0f)*(d/pLight[i].Range+1.0f));
            pDiffuse  *= 1.0f/((d/pLight[i].Range+1.0f)*(d/pLight[i].Range+1.0f));
            pSpec  *= 1.0f/((d/pLight[i].Range+1.0f)*(d/pLight[i].Range+1.0f));
        }
        if( d > softie*pLight[i].Range )
        {
            pAmbient *= 1.0f/((softie*pLight[i].Range/pLight[i].Range+1.0f)*(softie*pLight[i].Range/pLight[i].Range+1.0f));
            pAmbient *= (pLight[i].Range-d)/(pLight[i].Range-softie*pLight[i].Range);
            pDiffuse *= 1.0f/((softie*pLight[i].Range/pLight[i].Range+1.0f)*(softie*pLight[i].Range/pLight[i].Range+1.0f));
            pDiffuse *= (pLight[i].Range-d)/(pLight[i].Range-softie*pLight[i].Range);
            pSpec *= 1.0f/((softie*pLight[i].Range/pLight[i].Range+1.0f)*(softie*pLight[i].Range/pLight[i].Range+1.0f));
            pSpec *= (pLight[i].Range-d)/(pLight[i].Range-softie*pLight[i].Range);
        }
        
        [flatten]
        if(sceneBuff.pLightOn == 1.0f)
        {
            int val = 0;
            float shadow = 1.0;
            //if( 1 )
            //{
            if(sceneBuff.shadowsOn == 1.0f)
			{
				[flatten]
				if( i*6+6 <= NUMPOINTSHADOWS )
				{
					if( i == 0 )
						shadow = shadowValPoint( input, 0 + NUMDIRSHADOWS );
					else if( i == 1 )
						shadow = shadowValPoint( input, 6 + NUMDIRSHADOWS);
					else if( i == 2 )
						shadow = shadowValPoint( input, 12 + NUMDIRSHADOWS);
					else if( i == 3 )
						shadow = shadowValPoint( input, 18 + NUMDIRSHADOWS);
				}
			}

            if(sceneBuff.ambientOn == 1.0f)
                totalAmbient += (4.0f * pAmbient);
            if(sceneBuff.diffuseOn == 1.0f)
                totalDiffuse += (4.0f * pDiffuse) * 10 * shadow;
            if(sceneBuff.specularOn == 1.0f)
                totalSpec	 += pSpec;
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