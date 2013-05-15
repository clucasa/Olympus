#include "apex.h"

Apex::Apex() :
    mNbThreads(8),
    mPhysics(0),
    mFoundation(0),
    mCooking(0),
    mScene(0),
    mCurrentScene(0)
{
    return;
}

Apex::~Apex()
{
    gApexScene[mCurrentScene]->setPhysXScene(0);

    // Now, it's safe to release the NxScene...
    gApexScene[mCurrentScene]->fetchResults(true, NULL);                 // ensure scene is not busy
    gApexScene[mCurrentScene]->release();
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
    while (dt > mStepSize)
    {
        gApexScene[mCurrentScene]->simulate(mStepSize);
        gApexScene[mCurrentScene]->fetchResults(false, NULL);
        dt -= mStepSize;
    }
    gApexScene[mCurrentScene]->simulate(dt);
    gApexScene[mCurrentScene]->fetchResults(true, NULL);
    return true;
}

void Apex::fetch()
{
    //gApexScene[mCurrentScene]->fetchResults(true, NULL);
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
    
 //   NxApexSceneDesc apexSceneDesc;
 //   // Create the APEX scene...
 //   
 //   apexSceneDesc.scene = mScene[mCurrentScene];
    //
 //   if(apexSceneDesc.isValid())
 //       gApexScene[mCurrentScene] = gApexSDK->createScene(apexSceneDesc);
 //   else
 //       return false;

 //   if(!gApexScene[mCurrentScene])
 //       return false;

    //gApexScene[mCurrentScene]->setLODResourceBudget(10000.f);

    //static const physx::PxU32 viewIDlookAtRightHand = gApexScene[mCurrentScene]->allocViewMatrix(physx::apex::ViewMatrixType::LOOK_AT_LH);
    //static const physx::PxU32 projIDperspectiveCubicRightHand = gApexScene[mCurrentScene]->allocProjMatrix(physx::apex::ProjMatrixType::USER_CUSTOMIZED);

    //gApexScene[mCurrentScene]->setUseViewProjMatrix(viewIDlookAtRightHand, projIDperspectiveCubicRightHand);

    return true;
}

void Apex::UpdateViewProjMat(XMMATRIX *view, XMMATRIX *proj, float nearPlane, float farPlane, float fov, float vWidth, float vHeight, int scene)
{
    PxMat44 pview;
    XMtoPxMatrix(view, &pview);

    PxMat44 pproj;
    XMtoPxMatrix(proj, &pproj);

    gApexScene[scene]->setViewMatrix(pview);
    gApexScene[scene]->setProjMatrix(pproj);

    gApexScene[scene]->setProjParams(nearPlane, farPlane, fov, (PxU32)vWidth, (PxU32)vHeight);
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
    

    //PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    //sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

    //if(!sceneDesc.cpuDispatcher)
    //{
    //    mCpuDispatcher = PxDefaultCpuDispatcherCreate(mNbThreads);
    //    if(!mCpuDispatcher)
    //        return false;
    //    sceneDesc.cpuDispatcher    = mCpuDispatcher;
    //}

    //if(!sceneDesc.filterShader)
    //{
    //    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    //}
    //
    ///*#ifdef PX_WINDOWS
    //if(!sceneDesc.gpuDispatcher && mCudaContextManager)
    //{
    //    sceneDesc.gpuDispatcher = mCudaContextManager->getGpuDispatcher();
    //}
    //#*/
    //mProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(mFoundation);
    //pxtask::CudaContextManagerDesc cudaContextManagerDesc;
    //mCudaContextManager = pxtask::createCudaContextManager(*mFoundation,cudaContextManagerDesc, mProfileZoneManager);
    //sceneDesc.gpuDispatcher = mCudaContextManager->getGpuDispatcher();


    //mScene[mCurrentScene] = mPhysics->createScene(sceneDesc);
    //if (!mScene[mCurrentScene])
    //    return false;
    
    defaultMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);    //static friction, dynamic friction, restitution
    if(!defaultMaterial)
        return false;

    // Create a plane
    PxRigidStatic* plane = PxCreatePlane(*mPhysics, PxPlane(PxVec3(0,1,0), 700), *defaultMaterial);
    if (!plane)
        return false;

    //mScene[mCurrentScene]->addActor(*plane);

    // Create a heightfield
    PhysXHeightfield* heightfield = new PhysXHeightfield();
    //heightfield->InitHeightfield(mPhysics, mScene[mCurrentScene], "terrain5.raw");

    // check if PvdConnection manager is available on this platform
    if(mPhysics->getPvdConnectionManager() == NULL)
    {
        return true;
    }

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
        meshShape->setLocalPose(PxTransform(PxVec3(info.x,info.y,info.z), PxQuat(info.ry, PxVec3(0.0f,1.0f,0.0f))));
        meshShape->setFlag(PxShapeFlag::eUSE_SWEPT_BOUNDS, true);


        meshShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
        

        mScene[mCurrentScene]->addActor(*meshActor);
    }
}


static const PxVec3 convexVerts[] = {PxVec3(1,1,0),PxVec3(-1,1,0),PxVec3(0,1,1),PxVec3(0,1,-1),PxVec3(1,-1,0),PxVec3(-1,-1,0),PxVec3(0,-1,1),PxVec3(0,-1,-1)};

void Apex::LoadDynamicTriangleMesh(int numVerts, PxVec3* verts, ObjectInfo info)
{
    PxRigidDynamic* meshActor = mPhysics->createRigidDynamic(PxTransform::createIdentity());
    PxShape* meshShape, *convexShape;
    if(meshActor)
    {
        //meshActor->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);

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
        //meshShape->setLocalPose(PxTransform(PxVec3(info.x,info.y,info.z)));
        meshShape->setFlag(PxShapeFlag::eUSE_SWEPT_BOUNDS, true);

        PxConvexMeshDesc convexDesc;
        convexDesc.points.count     = numVerts;
        convexDesc.points.stride    = sizeof(PxVec3);
        convexDesc.points.data      = verts;
        convexDesc.flags            = PxConvexFlag::eCOMPUTE_CONVEX;

        if(!convexDesc.isValid())
            return;
        PxToolkit::MemoryOutputStream buf;
        if(!mCooking->cookConvexMesh(convexDesc, buf))
            return;
        PxToolkit::MemoryInputData input(buf.getData(), buf.getSize());
        PxConvexMesh* convexMesh = mPhysics->createConvexMesh(input);
        PxConvexMeshGeometry convexGeom = PxConvexMeshGeometry(convexMesh);
        convexGeom.scale = PxMeshScale(PxVec3(info.sx,info.sy,info.sz),physx::PxQuat::createIdentity());
        convexShape = meshActor->createShape(convexGeom, *defaultMaterial);
        //convexShape->setLocalPose(PxTransform(PxVec3(info.x,info.y,info.z)));
        //convexShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
        //meshActor->se
        
        convexShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
        meshShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
        meshActor->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, false);

        meshActor->setMass(50.0f);

        meshActor->setGlobalPose(PxTransform(PxVec3(info.x,info.y,info.z), PxQuat(info.ry, PxVec3(0.0f,1.0f,0.0f))));
        mScene[mCurrentScene]->addActor(*meshActor);
        dynamicActors.push_back(meshActor);
    }
}

void Apex::getRigidDynamicPosition(int index, XMFLOAT4X4 *position)
{
    PxU32 nShapes = 0;

    if(dynamicActors[index])
        nShapes = dynamicActors[index]->getNbShapes();
    else	
        return;

    PxShape** shapes = new PxShape*[nShapes];
 
    dynamicActors[index]->getShapes(shapes, nShapes);     
    PxTransform pt = PxShapeExt::getGlobalPose(*shapes[0]);

    delete [] shapes;

    XMMATRIX world = XMLoadFloat4x4(position) ;
    PxtoXMMatrix(pt, &world);

    XMStoreFloat4x4(position, world);
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
        mParticleIosModule->setLODUnitCost(0.00000001f);
    }
    
    mIofxModule = static_cast<NxModuleIofx*>(gApexSDK->createModule("IOFX", &errorCode));
    checkErrorCode(&errorCode);
    PX_ASSERT(mIofxModule);
    if (mIofxModule)
    {
        NxParameterized::Interface* params = mIofxModule->getDefaultModuleDesc();
        mIofxModule->init(*params);
        /*mIofxModule->disableCudaInterop();
        mIofxModule->disableCudaModifiers();*/
        mIofxModule->setLODUnitCost(0.00000001f);
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
            mEmitterModule->setLODUnitCost(0.00000001f);
        }
    }

    return true;
}

ApexParticles* Apex::CreateEmitter(physx::apex::NxUserRenderer* renderer, const char* filename, const char* texfile)
{
    ApexParticles* emitter = new ApexParticles();
    emitter->CreateEmitter(gApexSDK, gApexScene[mCurrentScene], mDevcon, mDev, renderer, mIofxModule, filename, texfile);
    return emitter;
}

ApexCloth* Apex::CreateCloth(physx::apex::NxUserRenderer* renderer, const char* filename, const char* texfile, float maxWind)
{
    ApexCloth* cloth = new ApexCloth(maxWind);
    cloth->CreateCloth(gApexSDK, gApexScene[mCurrentScene], mDevcon, mDev, renderer, filename, texfile);
    return cloth;
}

void Apex::CreatePlane(float nx, float ny, float nz, float distance)
{
    // Create a plane
    PxRigidStatic* plane = PxCreatePlane(*mPhysics, PxPlane(PxVec3(nx,ny,nz), distance), *defaultMaterial);
    if (!plane)
        return;

    gApexScene[mCurrentScene]->getPhysXScene()->addActor(*plane);
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
        mApexClothingModule->setLODUnitCost(0.0001f);
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

bool Apex::CreateScene(float gravity)
{
    PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, gravity, 0.0f);

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
    mProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(mFoundation);
    pxtask::CudaContextManagerDesc cudaContextManagerDesc;
    mCudaContextManager = pxtask::createCudaContextManager(*mFoundation,cudaContextManagerDesc, mProfileZoneManager);
    sceneDesc.gpuDispatcher = mCudaContextManager->getGpuDispatcher();


    mScene.push_back(mPhysics->createScene(sceneDesc));

    if (!mScene[mCurrentScene])
        return false;


     NxApexSceneDesc apexSceneDesc;
    // Create the APEX scene...
    
    apexSceneDesc.scene = mScene[mCurrentScene];
    
    if(apexSceneDesc.isValid())
        gApexScene.push_back(gApexSDK->createScene(apexSceneDesc));
    else
        return false;

    if(!gApexScene[mCurrentScene])
        return false;

    gApexScene[mCurrentScene]->setLODResourceBudget(10000.f);

    static const physx::PxU32 viewIDlookAtRightHand = gApexScene[mCurrentScene]->allocViewMatrix(physx::apex::ViewMatrixType::LOOK_AT_LH);
    static const physx::PxU32 projIDperspectiveCubicRightHand = gApexScene[mCurrentScene]->allocProjMatrix(physx::apex::ProjMatrixType::USER_CUSTOMIZED);

    gApexScene[mCurrentScene]->setUseViewProjMatrix(viewIDlookAtRightHand, projIDperspectiveCubicRightHand);
    return true;
}