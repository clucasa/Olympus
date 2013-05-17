#include "ApexCloth.h"


ApexCloth::ApexCloth(float maxWind) : mMaxWind(maxWind)
{
    return;
}

ApexCloth::~ApexCloth()
{
    return;
}

void ApexCloth::InitPipeline()
{
    ID3D10Blob* pErrorBlob = NULL;
    LPVOID pError = NULL;
    char* errorStr = NULL;
    // compile the shaders
    ID3D10Blob *sVS, *sPS;

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
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "NORMAL",	  0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // use the input element descriptions to create the input layout
    mDev->CreateInputLayout(clothied, 3, sVS->GetBufferPointer(), sVS->GetBufferSize(), &mLayout);

    // create the constant buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    // Create sprite constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 80;    // 4 for each float, float 4x4 = 4 * 4 * 4 + float 3 eyepos
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    hr = mDev->CreateBuffer(&bd, NULL, &mConstBuffer);
}


void ApexCloth::CreateCloth(NxApexSDK* gApexSDK, NxApexScene* gApexScene,
                        ID3D11DeviceContext *devcon, 	ID3D11Device *dev,
                      physx::apex::NxUserRenderer* renderer, const char* filename, const char* texfile)
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
    NxParameterized::setParamVec3(*actorDesc, "windParams.Velocity", PxVec3(2.0,0.0,17.0));
    NxParameterized::setParamF32(*actorDesc, "windParams.Adaption", 0.25f);


    // Initialize the global pose
    PxMat44 currentPose = PxTransform(PxVec3(0.0f, 0.0f, 0.0f), PxQuat(PxHalfPi, PxVec3(-1.,0.,0.) ) );//-40.f,-12.f,-39.f));
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
        
        //clothingActor->forcePhysicalLod(0);
        NxParameterized::Interface* actorDesc = clothingActor->getActorDesc();
        //clothingActor->createCollisionPlane(PxPlane(0,1,0,-1));
        //clothingActor->createCollisionSphere(
        NxParameterized::setParamVec3(*actorDesc, "windParams.Velocity", PxVec3(2.0,0.0,17.0));
        NxParameterized::setParamF32(*actorDesc, "windParams.Adaption", 0.25f);

        NxParameterized::setParamF32(*actorDesc, "lodWeights.maxDistance", 10000.0f);
        NxParameterized::setParamF32(*actorDesc, "lodWeights.distanceWeight", 100.0);
        NxParameterized::setParamF32(*actorDesc, "lodWeights.benefitsBias", 10.0);

    }

    std::string tfile = std::string("Media/Textures/") + texfile;
    HRESULT hre = D3DX11CreateShaderResourceViewFromFile(mDev, tfile.c_str(), 0, 0, &clothTexture, 0 );

    InitPipeline();
}


void ApexCloth::Update()
{
    float low = -mMaxWind;
    float high = mMaxWind;
    float valx = low + (float)rand()/((float)RAND_MAX/(high-low));
    float valy = 0.0f;
    float valz = abs(low + (float)rand()/((float)RAND_MAX/(high-low)));
 
    NxParameterized::Interface* actorDesc = clothingActor->getActorDesc();
    NxParameterized::setParamVec3(*actorDesc, "windParams.Velocity", PxVec3((PxReal)valx,(PxReal)valy,(PxReal)valz));
    
    clothingActor->lockRenderResources();
    clothingActor->updateRenderResources();
    clothingActor->unlockRenderResources();

    
}

void ApexCloth::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{
    mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->PSSetShader(mPS, 0, 0);
    mDevcon->PSSetShaderResources(0, 1, &clothTexture);
    mDevcon->IASetInputLayout(mLayout);

    clothingActor->dispatchRenderResources(*gRenderer);
}

void ApexCloth::Depth()
{
    mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->PSSetShader(NULL, 0, 0);
    mDevcon->PSSetShaderResources(0, 1, &clothTexture);
    mDevcon->IASetInputLayout(mLayout);

    clothingActor->dispatchRenderResources(*gRenderer);
}

void ApexCloth::SetPosition(float x, float y, float z, float rx, float ry, float rz)
{
    PxMat44 currentPose = PxTransform(PxVec3(x, y, z), PxQuat(ry, PxVec3(0.,1.,0.) ) );//-40.f,-12.f,-39.f));//0.f,-5.f,5.f));//
    clothingActor->updateState(currentPose, NULL, 0, 0, ClothingTeleportMode::Continuous);
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