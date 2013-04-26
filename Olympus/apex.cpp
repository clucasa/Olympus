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

	while (dt > mStepSize)
	{
		gApexScene->simulate(mStepSize);
        dt -= mStepSize;
	}
    gApexScene->simulate(dt);
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

	static const physx::PxU32 viewIDlookAtRightHand = gApexScene->allocViewMatrix(physx::apex::ViewMatrixType::LOOK_AT_LH);
	static const physx::PxU32 projIDperspectiveCubicRightHand = gApexScene->allocProjMatrix(physx::apex::ProjMatrixType::USER_CUSTOMIZED);

	gApexScene->setUseViewProjMatrix(viewIDlookAtRightHand, projIDperspectiveCubicRightHand);

    return true;
}

void Apex::UpdateViewProjMat(XMMATRIX *view, XMMATRIX *proj, float nearPlane, float farPlane, float fov, float vWidth, float vHeight)
{
	PxMat44 pview;
	XMtoPxMatrix(view, &pview);

	PxMat44 pproj;
	XMtoPxMatrix(proj, &pproj);

	gApexScene->setViewMatrix(pview);
	gApexScene->setProjMatrix(pproj);

	gApexScene->setProjParams(nearPlane, farPlane, fov, vWidth, vHeight);
}

void Apex::PxtoXMMatrix(PxTransform input, XMMATRIX* start)
{
	PxMat33 quat = PxMat33(input.q);

	start->_11 = quat.column0[0];
	start->_12 = quat.column0[1];
	start->_13 = quat.column0[2];


	start->_21 = quat.column1[0];
	start->_22 = quat.column1[1];
	start->_23 = quat.column1[2];


	start->_31 = quat.column2[0];
	start->_32 = quat.column2[1];
	start->_33 = quat.column2[2];


	start->_41 = input.p.x;
	start->_42 = input.p.y;
	start->_43 = input.p.z; 
}

void Apex::XMtoPxMatrix(XMMATRIX* input, PxMat44* start)
{
	start->column0.w = input->_11;
	start->column0.x = input->_12;
	start->column0.y = input->_13;
	start->column0.z = input->_14;

	start->column1.w = input->_21;
	start->column1.x = input->_22;
	start->column1.y = input->_23;
	start->column1.z = input->_24;

	start->column2.w = input->_31;
	start->column2.x = input->_32;
	start->column2.y = input->_33;
	start->column2.z = input->_34;

	start->column3.w = input->_41;
	start->column3.x = input->_42;
	start->column3.y = input->_43;
	start->column3.z = input->_44;
    
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

    //mScene->addActor(*plane);

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

void Apex::LoadTriangleMesh(int numVerts, PxVec3* verts, ObjectInfo info)
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
			triGeom.scale = PxMeshScale(PxVec3(info.sx,info.sy,info.sz),physx::PxQuat::createIdentity());
		
			meshShape = meshActor->createShape(triGeom, *defaultMaterial);
			meshShape->setLocalPose(PxTransform(PxVec3(info.x,info.y,info.z)));
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

bool Apex::InitClothing()
{
  
    PX_ASSERT(gApexSDK);
    NxApexCreateError            errorCode;
    mApexClothingModule = static_cast<physx::apex::NxModuleClothing*>(gApexSDK->createModule("Clothing", &errorCode));

    if (mApexClothingModule != NULL)
    {
        NxParameterized::Interface* moduleDesc = mApexClothingModule->getDefaultModuleDesc();

        // Know what you're doing when playing with these values!

        // should not be 0 for every platform except PC.
        NxParameterized::setParamU32(*moduleDesc, "maxNumCompartments", 3);

        // Can be tuned for switching between more memory and more spikes.
        NxParameterized::setParamU32(*moduleDesc, "maxUnusedPhysXResources", 5);

        mApexClothingModule->init(*moduleDesc);
    }

    physx::apex::NxApexAsset* asset = reinterpret_cast<physx::apex::NxApexAsset*>(gApexSDK->getNamedResourceProvider()->getResource(NX_CLOTHING_AUTHORING_TYPE_NAME, "curtain.mesh"));
    if( asset )
    {
        gApexSDK->getNamedResourceProvider()->setResource(NX_CLOTHING_AUTHORING_TYPE_NAME, "c", asset, true);
    }


    return true;
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