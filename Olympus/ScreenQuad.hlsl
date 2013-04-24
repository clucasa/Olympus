cbuffer ViewProjMat	: register(c0)
{
    float4x4 ViewProj;
}

cbuffer ObjectMat	: register(c1)
{
    float4x4 viewInvProj;
	float4x4 viewPrev;

	float	 zNear;
	float	 zFar;
	float2	 padding;
}

Texture2D tex : register(t0);
Texture2D depth : register(t1);
Texture2D shadow : register(t6);

struct VOut
{
	float4 posH : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

VOut VShader(float3 position : POSITION, float3 normal : NORMAL, float3 tangent : TANGENT, float2 texcoord : TEXCOORD )
{
    VOut output;

	output.posH = mul(ViewProj, float4(position, 1.0f));
    output.texcoord = texcoord;

    return output;
}


float4 PShader(VOut input) : SV_TARGET
{   
	float gamma = 1.5f;
	float lumt = 1.6f;
	float luminance;
    //return depth.Sample(samLinear, input.texcoord).r;
	//return tex.Sample(samLinear, input.texcoord);
    //return shadow.Sample(samLinear, input.texcoord);

	float4 color = depth.Sample( samLinear, input.texcoord );
	color.r = 2*zFar*zNear / (zFar + zNear - (zFar - zNear)*(2*color.r -1));

	float z_b = depth.Sample( samLinear, float2( .5, .5 ) ).r;
	float midDepth = 2*zFar*zNear / (zFar + zNear - (zFar - zNear)*(2*z_b -1));
	float blurFactor = 1.0;

	float depthRange = .008 * (zFar - zNear );

	if( color.r > midDepth - depthRange && color.r < midDepth + depthRange )
	{
		color = tex.Sample( samLinear, input.texcoord );
		// Luminance
		luminance = 0.2125*color.r + 0.7154*color.g + 0.0721*color.b;
		color = float4((1. - lumt) * float3(luminance, luminance, luminance) + lumt * color.rgb, 1.0f);
		// Gamma correction (Must be done last)
		color = pow(color, 1.0f / gamma);

		return color;
	}
	else
	{
		if( abs( midDepth + depthRange - color.r ) > abs( midDepth - depthRange - color.r ) )
			blurFactor =  ( ( midDepth - depthRange - color.r ) - zNear ) / ( zFar - zNear );
		else
			blurFactor =  ( ( midDepth + depthRange - color.r ) - zNear ) / ( zFar - zNear );

		blurFactor = abs( blurFactor );
	}

	color = (tex.Sample( samLinear, input.texcoord )) * 8.0f;
	//return float4(1.0,0.0,0.0,1.0);
	float blur = .003;
	
	color += ( tex.Sample( samLinear, float2( input.texcoord.x+blur, input.texcoord.y ) ) ) * 2.0f;
	color += ( tex.Sample( samLinear, float2( input.texcoord.x-blur, input.texcoord.y ) ) ) * 2.0f;
	color += ( tex.Sample( samLinear, float2( input.texcoord.x, input.texcoord.y+blur ) ) ) * 2.0f;
	color += ( tex.Sample( samLinear, float2( input.texcoord.x, input.texcoord.y-blur ) ) ) * 2.0f;

	color += tex.Sample( samLinear, float2( input.texcoord.x-blur, input.texcoord.y-blur ) );
	color += tex.Sample( samLinear, float2( input.texcoord.x+blur, input.texcoord.y-blur ) );
	color += tex.Sample( samLinear, float2( input.texcoord.x-blur, input.texcoord.y+blur ) );
	color += tex.Sample( samLinear, float2( input.texcoord.x+blur, input.texcoord.y+blur ) );

	color = color / 20.0f;

	// Locals

	//float BlurStrength = 0.3f;
	//float BlurScale = 1.0f;

 //   float halfBlur = float(BlurAmount) * 0.5;
 //   vec4 colour = vec4(0.0);
 //   vec4 texColour = vec4(0.0);
 //   
 //   // Gaussian deviation
 //   float deviation = halfBlur * 0.35;
 //   deviation *= deviation;
 //   float strength = 1.0 - BlurStrength;
 //   
 //   if ( Orientation == 0 )
 //   {
 //       // Horizontal blur
 //       for (int i = 0; i < 10; ++i)
 //       {
 //           if ( i >= BlurAmount )
 //               break;
 //           
 //           float offset = float(i) - halfBlur;
 //           texColour = texture2D(Sample0, vUv + vec2(offset * TexelSize.x * BlurScale, 0.0)) * Gaussian(offset * strength, deviation);
 //           colour += texColour;
 //       }
 //   }

	// Luminance
	luminance = 0.2125*color.r + 0.7154*color.g + 0.0721*color.b;
	color = float4((1. - lumt) * float3(luminance, luminance, luminance) + lumt * color.rgb, 1.0f);
	// Gamma correction (Must be done last)
	color = pow(color, 1.0f / gamma);

	color.a = 1.0f;
	return color;
}

/// <summary>
/// Gets the Gaussian value in the first dimension.
/// </summary>
/// <param name="x">Distance from origin on the x-axis.</param>
/// <param name="deviation">Standard deviation.</param>
/// <returns>The gaussian value on the x-axis.</returns>
float Gaussian (float x, float deviation)
{
    return (1.0 / sqrt(2.0 * 3.141592 * deviation)) * exp(-((x * x) / (2.0 * deviation)));  
}