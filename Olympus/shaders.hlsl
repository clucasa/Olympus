cbuffer ConstantBuffer
{
    float4x4 final;
    float4x4 rotation;    // the rotation matrix
    float4 lightvec;      // the light's vector
    float4 lightcol;      // the light's color
    float4 ambientcol;    // the ambient light's color
}

Texture2D Texture;
SamplerState ss;

struct VOut
{
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;    // texture coordinates
    float4 position : SV_POSITION;
};

VOut VShader(float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD)
{
    VOut output;

    output.position = mul(final, position);

    // set the ambient light
    output.color = ambientcol;

    // calculate the diffuse light and add it to the ambient light
    float4 norm = normalize(mul(rotation, normal));
    float diffusebrightness = saturate(dot(norm, lightvec));
    output.color += lightcol * diffusebrightness;

    output.texcoord = texcoord;    // set the texture coordinates, unmodified

    return output;
}

float4 PShader(float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
    return color * Texture.Sample(ss, texcoord);
}
