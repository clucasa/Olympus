#include "ApexCloth.h"


ApexCloth::ApexCloth()
{
    
    InitPipeline();

    return;
}

ApexCloth::~ApexCloth()
{
    return;
}

void ApexCloth::CreateCloth(NxApexSDK* gApexSDK, NxApexScene* gApexScene,
						ID3D11DeviceContext *devcon, 	ID3D11Device *dev,
					  physx::apex::NxUserRenderer* renderer, const char* filename)
{
    mDev = dev;
	mDevcon = devcon;
	gRenderer = renderer;

    NxClothingAsset* clothAsset;
    physx::apex::NxApexAsset* asset = reinterpret_cast<physx::apex::NxApexAsset*>(gApexSDK->getNamedResourceProvider()->getResource(NX_CLOTHING_AUTHORING_TYPE_NAME, filename/*"ctdm_Cape_400"*/));
    if( asset )
    {
        gApexSDK->getNamedResourceProvider()->setResource(NX_CLOTHING_AUTHORING_TYPE_NAME, "c", asset, true);
        clothAsset = static_cast<NxClothingAsset*>(asset);
    }
    gApexSDK->forceLoadAssets();

    // Get the (singleton!) default actor descriptor.
    NxParameterized::Interface* actorDesc = clothAsset->getDefaultActorDesc();
    PX_ASSERT(actorDesc != NULL);

    // Run Cloth on the GPU
    NxParameterized::setParamBool(*actorDesc, "useHardwareCloth", true);
    NxParameterized::setParamBool(*actorDesc, "flags.ParallelPhysXMeshSkinning", true);

    // Initialize the global pose
    PxMat44 currentPose = PxTransform(PxVec3(0.f,5.f,0.f));
    NxParameterized::setParamMat44(*actorDesc, "globalPose", currentPose);

    //{
    //    NxParameterized::Handle actorHandle(*actorDesc);

    //    // No util method for this
    //    actorHandle.getParameter("boneMatrices");
    //    actorHandle.resizeArray(skinningMatrices.size());
    //    actorHandle.setParamMat44Array(&skinningMatrices[0], skinningMatrices.size());
    //}

    // create the actor
    physx::apex::NxApexActor* apexActor = clothAsset->createApexActor(*actorDesc, *gApexScene);
    clothingActor = static_cast<physx::apex::NxClothingActor*>(apexActor);
    if(clothingActor)
    {
        NxParameterized::Interface* actorDesc = clothingActor->getActorDesc();

        NxParameterized::setParamVec3(*actorDesc, "windParams.Velocity", PxVec3(1.0,0.0,1.0));
        NxParameterized::setParamF32(*actorDesc, "windParams.Adaption", 0.25f);
    }
}

void ApexCloth::InitPipeline()
{

}

void ApexCloth::Update()
{
    clothingActor->lockRenderResources();
    clothingActor->updateRenderResources();
    clothingActor->unlockRenderResources();
}

void ApexCloth::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{
    
    clothingActor->dispatchRenderResources(*gRenderer);
}


void ApexCloth::RecompileShader()
{
	//// compile the shaders
 //   ID3D10Blob *sVS, *sPS, *sGS;
	//HRESULT hr;
	//hr = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &sVS, 0, 0);

 //   hr = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "GShader", "gs_5_0", 0, 0, 0, &sGS, 0, 0);

 //   hr = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &sPS, 0, 0);


 //   // create the shader objects

 //   mDev->CreateVertexShader(sVS->GetBufferPointer(), sVS->GetBufferSize(), NULL, &mVS);
 //   mDev->CreateGeometryShader(sGS->GetBufferPointer(), sGS->GetBufferSize(), NULL, &mGS);
 //   mDev->CreatePixelShader(sPS->GetBufferPointer(), sPS->GetBufferSize(), NULL, &mPS);
    

}