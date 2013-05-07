#include "ApexDestructable.h"


ApexDestructable::ApexDestructable()
{
    
   

    return;
}

ApexDestructable::~ApexDestructable()
{
    return;
}

void ApexDestructable::InitPipeline()
{
	ID3D10Blob* pErrorBlob = NULL;
    LPVOID pError = NULL;
    char* errorStr = NULL;
	// compile the shaders
    ID3D10Blob *sVS, *sPS, *sGS;

	HRESULT hr = D3DX11CompileFromFile("clothshader.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &sVS, &pErrorBlob, 0);
    if(pErrorBlob)
    {
        pError = pErrorBlob->GetBufferPointer();
        errorStr = (char*)pError;
        __asm {
            INT 3
        }
        return;
    }

    hr = D3DX11CompileFromFile("clothshader.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &sPS, &pErrorBlob, 0);
    if(pErrorBlob)
    {
        pError = pErrorBlob->GetBufferPointer();
        errorStr = (char*)pError;
        __asm {
            INT 3
        }
        return;
    }

    // create the shader objects

    mDev->CreateVertexShader(sVS->GetBufferPointer(), sVS->GetBufferSize(), NULL, &mVS);
    mDev->CreatePixelShader(sPS->GetBufferPointer(), sPS->GetBufferSize(), NULL, &mPS);

	// create the input element object
    D3D11_INPUT_ELEMENT_DESC clothied[] =
    {
       // { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		//{ "NORMAL",	  0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

    // use the input element descriptions to create the input layout
    mDev->CreateInputLayout(clothied, 1, sVS->GetBufferPointer(), sVS->GetBufferSize(), &mLayout);

    // create the constant buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

	// Create sprite constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 80;    // 4 for each float, float 4x4 = 4 * 4 * 4 + float 3 eyepos
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    hr = mDev->CreateBuffer(&bd, NULL, &mConstBuffer);
}


void ApexDestructable::CreateDestructable(NxApexSDK* gApexSDK, NxApexScene* gApexScene,
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
    NxParameterized::setParamBool(*actorDesc, "useHardwareCloth", false);
    NxParameterized::setParamBool(*actorDesc, "flags.ParallelPhysXMeshSkinning", false);

    // Initialize the global pose
    PxMat44 currentPose = PxTransform(PxVec3(0.f,5.f,0.f));
    NxParameterized::setParamMat44(*actorDesc, "globalPose", currentPose);

    {
        NxParameterized::Handle actorHandle(*actorDesc);

        // No util method for this
        actorHandle.getParameter("boneMatrices");
        actorHandle.resizeArray(0);
        //actorHandle.setParamMat44Array(&skinningMatrices[0], skinningMatrices.size());
    }

    // create the actor
    physx::apex::NxApexActor* apexActor = clothAsset->createApexActor(*actorDesc, *gApexScene);
    clothingActor = static_cast<physx::apex::NxClothingActor*>(apexActor);
    if(clothingActor)
    {
        NxParameterized::Interface* actorDesc = clothingActor->getActorDesc();

        NxParameterized::setParamVec3(*actorDesc, "windParams.Velocity", PxVec3(2.0,0.0,2.0));
        NxParameterized::setParamF32(*actorDesc, "windParams.Adaption", 0.25f);
    }

     InitPipeline();
}


void ApexDestructable::Update()
{
    clothingActor->lockRenderResources();
    clothingActor->updateRenderResources();
    clothingActor->unlockRenderResources();
}

void ApexDestructable::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{
    //SPRITECBUFFER scBuffer;
	//scBuffer.EyePos = mCam->GetPosition();

	mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->PSSetShader(mPS, 0, 0);
	mDevcon->IASetInputLayout(mLayout);

//    mDevcon->UpdateSubresource(mConstBuffer, 0, 0, &scBuffer, 0, 0);

    clothingActor->dispatchRenderResources(*gRenderer);
}


void ApexDestructable::RecompileShader()
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