#pragma once

#ifndef APEX_H
#define APEX_H
#define WIN32
#include "PxPhysicsAPI.h"
#include "PxVisualDebugger.h"
#include "PxVisualDebuggerExt.h"
#include "PvdNetworkStreams.h"
#include "pxtask/PxCudaContextManager.h"

#include < PxToolkit.h >
#include "PhysXHeightField.h"

#include "NxApex.h"
//#include "NxApexSDK.h"
#include "ZeusRenderResourceManager.h"
#include "ApexParticles.h"
#include "ApexCloth.h"

//#include "Object.h"
#include "ZeusResourceCallback.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <vector>
#include "LightHelper.h"

//using namespace std;
using namespace physx;
using namespace debugger;

#pragma comment(lib, "PhysX3CHECKED_x86.lib")
#pragma comment(lib, "PhysX3CommonCHECKED_x86.lib") 
#pragma comment(lib, "PhysX3CookingCHECKED_x86.lib") 
#pragma comment(lib, "PhysX3ExtensionsCHECKED.lib")
#pragma comment(lib, "PhysXVisualDebuggerSDKCHECKED.lib")
#pragma comment(lib, "PxTaskCHECKED.lib")
#pragma comment(lib, "PhysXProfileSDKCHECKED.lib")

#pragma comment(lib, "PxToolkitDEBUG.lib")

#pragma comment(lib ,"ApexFrameworkCHECKED_x86")

#ifndef OBJECT_INFO
#define OBJECT_INFO
struct ObjectInfo
{
	float x,y,z;
	float sx,sy,sz;
	float rx, ry, rz;

	vector<Material> materials; //one per mesh
};
#endif

class Apex
{
	// APEX
public:
    Apex();
    ~Apex();

    bool Init(ID3D11Device* dev, ID3D11DeviceContext* devcon);
    bool InitParticles();
    bool InitClothing();

    ApexParticles* CreateEmitter(physx::apex::NxUserRenderer* renderer, const char* filename);
    ApexCloth* CreateCloth(physx::apex::NxUserRenderer* renderer, const char* filename);

    bool advance(float dt);
    void fetch();
	void UpdateViewProjMat(XMMATRIX* view, XMMATRIX* proj, float nearPlane, float farPlane, float fov, float vWidth, float vHeight);
	void PxtoXMMatrix(PxTransform input, XMMATRIX* start);
	void XMtoPxMatrix(XMMATRIX* input, PxMat44* start);

    void Render();


	

	bool checkErrorCode(NxApexCreateError* err);
private:
    NxApexSDK*                  gApexSDK;
    NxApexScene*                gApexScene;
    physx::apex::NxUserRenderResourceManager*	m_renderResourceManager;

    ApexParticles*				gApexParticles;
    
	NxModuleParticleIos*        mParticleIosModule;
    NxModuleEmitter*            mEmitterModule;
    NxModuleIofx*               mIofxModule;

    ApexCloth*                  gApexCloth;

    NxModuleClothing*           mApexClothingModule;

	ID3D11Device* mDev;
	ID3D11DeviceContext* mDevcon;
	
// PhysX
public:
	void LoadTriangleMesh(int numVerts, PxVec3* verts, ObjectInfo info);
	void LoadDynamicTriangleMesh(int numVerts, PxVec3* verts, ObjectInfo info);
	PxScene*	getScene() {return mScene;}
	PxPhysics*	getPhysics() {return mPhysics;}
	PxFoundation* getFoundation() {return mFoundation;}
	void getRigidDynamicPosition(int index, XMFLOAT4X4 *position);

    bool InitPhysX();
	
	vector<PxRigidActor*>		dynamicActors;
    PxFoundation*               mFoundation;
    PxPhysics*                  mPhysics;
    PxProfileZoneManager*       mProfileZoneManager;
    PxCooking*                  mCooking;
    PxScene*                    mScene;
    PxDefaultCpuDispatcher*     mCpuDispatcher;
    pxtask::CudaContextManager* mCudaContextManager;
    PxU32                       mNbThreads;
    PxMaterial*					defaultMaterial;
    PVD::PvdConnection*	        pvdConnection;
};

#endif