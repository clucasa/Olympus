//***************************************************************************************
// ApexParticles.cpp
//***************************************************************************************
#include "ApexParticles.h"

ApexParticles::ApexParticles() :
    mParticleIosModule(0),
    mEmitterModule(0),
    mIofxModule(0)
{
    return;
}

ApexParticles::~ApexParticles()
{
    return;
}

void ApexParticles::Init(NxApexSDK* gApexSDK)
{
    PX_ASSERT(gApexSDK);
    NxApexCreateError            errorCode;


    mParticleIosModule = static_cast<NxModuleParticleIos*>(gApexSDK->createModule("ParticleIOS", &errorCode));
    checkErrorCode(&errorCode);
    PX_ASSERT(mParticleIosModule);
    if(mParticleIosModule)
    {
        NxParameterized::Interface* params = mParticleIosModule->getDefaultModuleDesc();
        mParticleIosModule->init(*params);
    }

    
    mIofxModule = static_cast<NxModuleIofx*>(gApexSDK->createModule("IOFX", &errorCode));
    checkErrorCode(&errorCode);
    PX_ASSERT(mIofxModule);
    if (mIofxModule)
    {
        NxParameterized::Interface* params = mIofxModule->getDefaultModuleDesc();
        mIofxModule->init(*params);

        //m_apexIofxModule->disableCudaInterop();
        //m_apexIofxModule->disableCudaModifiers();
    }

    mEmitterModule = static_cast<NxModuleEmitter*> ( gApexSDK->createModule("Emitter", &errorCode));
    checkErrorCode(&errorCode);
    PX_ASSERT(mEmitterModule);
    if(mEmitterModule)
    {
        NxParameterized::Interface* params = mEmitterModule->getDefaultModuleDesc();
        mEmitterModule->init(*params);
        PxU32 numScalables = mEmitterModule->getNbParameters();
        NxApexParameter** m_emitterModuleScalables = mEmitterModule->getParameters();
        for (physx::PxU32 i = 0; i < numScalables; i++)
        {
            NxApexParameter& p = *m_emitterModuleScalables[i];
            mEmitterModule->setIntValue(i, p.range.maximum);
        }
    }

}

void ApexParticles::CreateEmitter(NxApexSDK* gApexSDK, NxApexScene* gApexScene)
{
    NxApexEmitterAsset* emitterAsset;
    physx::apex::NxApexAsset* asset = reinterpret_cast<physx::apex::NxApexAsset*>(gApexSDK->getNamedResourceProvider()->getResource(NX_APEX_EMITTER_AUTHORING_TYPE_NAME, "testSpriteEmitter4ParticleFluidIos"));
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
    NxApexEmitterActor* emitterActor;
    if(descParams->areParamsOK())
    {
        emitterActor = static_cast<NxApexEmitterActor*>(emitterAsset->createApexActor(*descParams,*gApexScene));
        if(emitterActor)
        {
            emitterActor->setCurrentPosition(PxVec3(0.0f, 1.0f, 0.0f));
            emitterActor->startEmit( true );
            //emitterActor->setLifetimeRange(physx::apex::NxRange<PxF32>(1,5));
            //emitterActor->setRateRange(physx::apex::NxRange<PxF32>(10, 10));
        }
    }

    PxBounds3 b;
    b.setInfinite();

    mRenderVolume = mIofxModule->createRenderVolume(*gApexScene, b, 0, true );
    emitterActor->setPreferredRenderVolume( mRenderVolume );
}

bool ApexParticles::RenderVolume(physx::apex::NxUserRenderer & renderer)
{
    mRenderVolume->lockRenderResources();
  
    mRenderVolume->updateRenderResources(false);
    mRenderVolume->dispatchRenderResources(renderer);
    
	mRenderVolume->unlockRenderResources();
	return true;
}

bool ApexParticles::checkErrorCode(NxApexCreateError* err)
{
    bool retval = false;
    switch(*err)
    {
    case APEX_CE_NO_ERROR:
        retval =  true;
        break;
    case APEX_CE_NOT_FOUND:
        retval =  false;
        break;
    case APEX_CE_WRONG_VERSION:
        retval = false;
        break;
    case APEX_CE_DESCRIPTOR_INVALID:
        retval = false;
        break;
    }
    return retval;
}