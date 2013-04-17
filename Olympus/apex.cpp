#include "apex.h"

Apex::Apex() :
    mNbThreads(8),
    mPhysics(0),
    mFoundation(0),
    mCooking(0),
    mScene(0)

{
    return;
}

Apex::~Apex()
{
    gApexScene->setPhysXScene(0);

    // Now, it's safe to release the NxScene...
    gApexScene->fetchResults(true, NULL);                 // ensure scene is not busy
    gApexScene->release();
    mCpuDispatcher->release();


    // remember to release the connection by manual in the end
    if (pvdConnection)
            pvdConnection->release();
    mPhysics->release();
    mFoundation->release();

    return;
}

float mAccumulator = 0.0f;
float mStepSize = 1.0f / 60.0f;
float mCooldown = 0.0f;

bool Apex::advance(float dt)
{
    mAccumulator  += dt;
    if(mAccumulator < mStepSize)
        return false;

    mAccumulator -= mStepSize;

    if(mCooldown > 0.0f)
        mCooldown -= mStepSize;

    gApexScene->simulate(mStepSize);
    return true;
}

void Apex::fetch()
{
    gApexScene->fetchResults(true, NULL);
}

bool Apex::Init(ID3D11Device* dev, ID3D11DeviceContext* devcon)
{
    if(!InitPhysX())
        return false;


	mDev = dev;
	mDevcon = devcon;
    // Init Apex
    static PxDefaultErrorCallback gDefaultErrorCallback;
    ZeusResourceCallback* rcallback = new ZeusResourceCallback();
    NxApexSDKDesc   apexDesc;
    apexDesc.outputStream = &gDefaultErrorCallback;
    apexDesc.resourceCallback = rcallback;
    apexDesc.physXSDK = mPhysics;
    apexDesc.cooking = mCooking;
    
    m_renderResourceManager = new ZeusRenderResourceManager(dev,devcon);
    apexDesc.renderResourceManager = m_renderResourceManager;

    if(apexDesc.isValid())
        gApexSDK = NxCreateApexSDK(apexDesc);
    else
        return false;
   
    if(!gApexSDK)
        return false;

    NxApexSceneDesc apexSceneDesc;
    // Create the APEX scene...
    
    apexSceneDesc.scene = mScene;
    if(apexSceneDesc.isValid())
        gApexScene = gApexSDK->createScene(apexSceneDesc);
    else
        return false;

    if(!gApexScene)
        return false;

	static const physx::PxU32 viewIDlookAtRightHand = gApexScene->allocViewMatrix(physx::apex::ViewMatrixType::LOOK_AT_RH);
	static const physx::PxU32 projIDperspectiveCubicRightHand = gApexScene->allocProjMatrix(physx::apex::ProjMatrixType::USER_CUSTOMIZED);

    return true;
}

bool Apex::InitPhysX()
{
    static PxDefaultErrorCallback gDefaultErrorCallback;
    static PxDefaultAllocator gDefaultAllocatorCallback;

    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
    if(!mFoundation)
        return false;

    bool recordMemoryAllocations = true;

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation,
                PxTolerancesScale(), recordMemoryAllocations);
    if(!mPhysics)
        return false;

    mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, PxCookingParams());
    if (!mCooking)
        return false;

    if (!PxInitExtensions(*mPhysics))
        return false;


    PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

    if(!sceneDesc.cpuDispatcher)
    {
        mCpuDispatcher = PxDefaultCpuDispatcherCreate(mNbThreads);
        if(!mCpuDispatcher)
            return false;
        sceneDesc.cpuDispatcher    = mCpuDispatcher;
    }

    if(!sceneDesc.filterShader)
    {
        sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    }
    
    /*#ifdef PX_WINDOWS
    if(!sceneDesc.gpuDispatcher && mCudaContextManager)
    {
        sceneDesc.gpuDispatcher = mCudaContextManager->getGpuDispatcher();
    }
    #*/

    mScene = mPhysics->createScene(sceneDesc);
    if (!mScene)
        return false;

    defaultMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);    //static friction, dynamic friction, restitution
    if(!defaultMaterial)
        return false;

    // Create a plane
    PxRigidStatic* plane = PxCreatePlane(*mPhysics, PxPlane(PxVec3(0,1,0), 0), *defaultMaterial);
    if (!plane)
        return false;

    mScene->addActor(*plane);

    // Create a heightfield
    PhysXHeightfield* heightfield = new PhysXHeightfield();
    //heightfield->InitHeightfield(mPhysics, mScene, "terrain5.raw");


    //// Create a box
    //PxReal density = 10.0f;
    //PxTransform transform(PxVec3(0.0, 8.0, 0.0) , PxQuat::createIdentity());
    //PxVec3 dimensions(1.5,1.5,1.5);
    //PxBoxGeometry geometry(dimensions);
    //PxRigidDynamic* boxActor = PxCreateDynamic(*mPhysics, transform, geometry, *defaultMaterial, density);
    //if (!boxActor)
    //    return false;
    //
    //boxActor->setLinearVelocity(PxVec3(0.0,0.0,0.0));
    //boxActor->setAngularDamping((PxReal)0.95);
    ////PxRigidBodyExt::updateMassAndInertia(*boxActor, density);

    //mScene->addActor(*boxActor);
        

    // check if PvdConnection manager is available on this platform
    if(mPhysics->getPvdConnectionManager() == NULL)
        return false;

    // setup connection parameters
    const char*     pvd_host_ip = "127.0.0.1";  // IP of the PC which is running PVD
    int             port        = 5425;         // TCP port to connect to, where PVD is listening
    unsigned int    timeout     = 100;          // timeout in milliseconds to wait for PVD to respond,
                                                // consoles and remote PCs need a higher timeout.
    PxVisualDebuggerConnectionFlags connectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();

    // and now try to connect
    pvdConnection = PxVisualDebuggerExt::createConnection(mPhysics->getPvdConnectionManager(),
        pvd_host_ip, port, timeout, connectionFlags);

    mPhysics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlags::eTRANSMIT_CONTACTS, true);

    return true;
}

void Apex::LoadTriangleMesh(int numVerts, PxVec3* verts, float scale)
{
	PxRigidStatic* meshActor = mPhysics->createRigidStatic(PxTransform::createIdentity());
	PxShape* meshShape;
	if(meshActor)
	{
			PxTriangleMeshDesc meshDesc;
			meshDesc.points.count           = numVerts;
			meshDesc.points.stride          = sizeof(PxVec3);
			meshDesc.points.data            = verts;

			//meshDesc.triangles.count        = numInds/3.;
			//meshDesc.triangles.stride       = 3*sizeof(int);
			//meshDesc.triangles.data         = inds;

			PxToolkit::MemoryOutputStream writeBuffer;
			bool status = mCooking->cookTriangleMesh(meshDesc, writeBuffer);
			if(!status)
				return;

			PxToolkit::MemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

			PxTriangleMeshGeometry triGeom;
			triGeom.triangleMesh = mPhysics->createTriangleMesh(readBuffer);
			triGeom.scale = PxMeshScale(PxVec3(scale,scale,scale),physx::PxQuat::createIdentity());

			meshShape = meshActor->createShape(triGeom, *defaultMaterial);
			meshShape->setLocalPose(PxTransform(PxVec3(0,0,0))); //x,y,z)));
			meshShape->setFlag(PxShapeFlag::eUSE_SWEPT_BOUNDS, true);

			mScene->addActor(*meshActor);
	}
}

bool Apex::InitParticles()
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

    return true;
}

ApexParticles* Apex::CreateEmitter(physx::apex::NxUserRenderer* renderer)
{
	ApexParticles* emitter = new ApexParticles();
	emitter->CreateEmitter(gApexSDK, gApexScene, mDevcon, mDev, renderer, mIofxModule);
	return emitter;
}


void Apex::Render()
{
    //gApexParticles->RenderVolume(*gRenderer);
}


bool Apex::checkErrorCode(NxApexCreateError* err)
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