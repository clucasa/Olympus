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
	float    padding;
};