//***************************************************************************************
// ApexParticles.cpp
//***************************************************************************************
#include "ApexParticles.h"

ApexParticles::ApexParticles()
{
    return;
}

ApexParticles::~ApexParticles()
{
    return;
}

void ApexParticles::CreateEmitter(NxApexSDK* gApexSDK, NxApexScene* gApexScene,
	ID3D11DeviceContext *devcon, ID3D11Device *dev,
	physx::apex::NxUserRenderer* renderer, NxModuleIofx* iofxModule, const char* filename)
{
	mDev = dev;
	mDevcon = devcon;
	mIofxModule = iofxModule;
	gRenderer = renderer;

    NxApexEmitterAsset* emitterAsset;
    physx::apex::NxApexAsset* asset = reinterpret_cast<physx::apex::NxApexAsset*>(gApexSDK->getNamedResourceProvider()->getResource(NX_APEX_EMITTER_AUTHORING_TYPE_NAME, filename));
    if (asset)
    {
        emitterAsset = static_cast<NxApexEmitterAsset*> (asset);
    }
    //NxApexEmitterAsset* emitterAsset = static_cast<NxApexEmitterAsset*> (gApexSDK->createAsset(asParams, "testMeshEmitter4ParticleIos.apb"));
    gApexSDK->forceLoadAssets();

    NxParameterized::Interface* descParams = emitterAsset->getDefaultActorDesc();
    PX_ASSERT(descParams);
    if (!descParams)
    {
        return;
    }

    // Set Actor pose
    //NxParameterized::setParamMat44( *descParams, "initialPose", pose );
    
    if(descParams->areParamsOK())
    {
        emitterActor = static_cast<NxApexEmitterActor*>(emitterAsset->createApexActor(*descParams,*gApexScene));
        if(emitterActor)
        {
            emitterActor->setCurrentPosition(PxVec3(0.0f, 20.0f, 0.0f));
            emitterActor->startEmit( true );
			//emitterActor->forcePhysicalLod(
            //emitterActor->setLifetimeRange(physx::apex::NxRange<PxF32>(1,5));
            //emitterActor->setRateRange(physx::apex::NxRange<PxF32>(10, 10));
        }
    }

    PxBounds3 b;
    b.setInfinite();

    mRenderVolume = mIofxModule->createRenderVolume(*gApexScene, b, 0, true );
    emitterActor->setPreferredRenderVolume( mRenderVolume );

	InitPipeline();
}

void ApexParticles::InitPipeline()
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(mDev, /*"Media/Textures/SoftParticle.dds"*/"Media/Textures/popcorn.png", 0, 0, &spriteTexture, 0 );

	// compile the shaders
    ID3D10Blob *sVS, *sPS, *sGS;

	hr = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &sVS, 0, 0);

    hr = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "GShader", "gs_5_0", 0, 0, 0, &sGS, 0, 0);

    hr = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &sPS, 0, 0);


    // create the shader objects

    mDev->CreateVertexShader(sVS->GetBufferPointer(), sVS->GetBufferSize(), NULL, &mVS);
    mDev->CreateGeometryShader(sGS->GetBufferPointer(), sGS->GetBufferSize(), NULL, &mGS);
    mDev->CreatePixelShader(sPS->GetBufferPointer(), sPS->GetBufferSize(), NULL, &mPS);

	// create the input element object
    D3D11_INPUT_ELEMENT_DESC spriteied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

    // use the input element descriptions to create the input layout

    mDev->CreateInputLayout(spriteied, 1, sVS->GetBufferPointer(), sVS->GetBufferSize(), &mLayout);


    // create the constant buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

	// Create sprite constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 80;    // 4 for each float, float 4x4 = 4 * 4 * 4 + float 3 eyepos
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    hr = mDev->CreateBuffer(&bd, NULL, &mConstBuffer);
}

struct SPRITECBUFFER
{
	XMFLOAT3 EyePos;
	float buffer;
};

void ApexParticles::Update()
{
	mRenderVolume->lockRenderResources();
	mRenderVolume->updateRenderResources(false);
	mRenderVolume->unlockRenderResources();
}

void ApexParticles::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{
	SPRITECBUFFER scBuffer;
	scBuffer.EyePos = mCam->GetPosition();

	mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->GSSetShader(mGS, 0, 0);
    mDevcon->PSSetShader(mPS, 0, 0);
	mDevcon->IASetInputLayout(mLayout);

	mDevcon->GSSetConstantBuffers(0, 1, &sceneBuff);
	mDevcon->GSSetConstantBuffers(1, 1, &mConstBuffer);
    mDevcon->PSSetShaderResources(0, 1, &spriteTexture);
	mDevcon->UpdateSubresource(mConstBuffer, 0, 0, &scBuffer, 0, 0);

	mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->PSSetShader(mPS, 0, 0);
    mDevcon->IASetInputLayout(mLayout);
		
	mRenderVolume->dispatchRenderResources(*gRenderer);
    
	mDevcon->GSSetShader(NULL, 0, 0);
}

void ApexParticles::SetPosition(float x, float y, float z)
{
	if(emitterActor)
    {
        emitterActor->setCurrentPosition(PxVec3(x, y, z));
    }
}

void ApexParticles::SetEmit(bool on)
{
	if(emitterActor)
    {
		if(on)
		{
			emitterActor->startEmit( on );
		}
		else
		{
		    emitterActor->stopEmit();
		}
    }
}


void ApexParticles::RecompileShader()
{
	// compile the shaders
    ID3D10Blob *sVS, *sPS, *sGS;
	HRESULT hr;
	hr = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &sVS, 0, 0);

    hr = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "GShader", "gs_5_0", 0, 0, 0, &sGS, 0, 0);

    hr = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &sPS, 0, 0);


    // create the shader objects

    mDev->CreateVertexShader(sVS->GetBufferPointer(), sVS->GetBufferSize(), NULL, &mVS);
    mDev->CreateGeometryShader(sGS->GetBufferPointer(), sGS->GetBufferSize(), NULL, &mGS);
    mDev->CreatePixelShader(sPS->GetBufferPointer(), sPS->GetBufferSize(), NULL, &mPS);
    

}