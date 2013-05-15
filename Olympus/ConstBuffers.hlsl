struct SceneBuffer
{
    float4x4 ViewProj;
    float3   cameraPos;
    float    phong;
    float    normalMap;
    float	 textures;
    float    ambientOn;
    float	 diffuseOn;
    float    specularOn;
    float    pLightOn;
    float    dirLightOn;
    float    shadowsOn;
};

struct ShadowProj
{
	float4x4 lightViewProj;
	float3 lightPos;
	float PADdyCake;
};