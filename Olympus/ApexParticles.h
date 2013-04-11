#pragma once
//***************************************************************************************
// ApexParticles.h
//
//
// 
//
//***************************************************************************************
#ifndef APEX_PARTICLES_H
#define APEX_PARTICLES_H

//#include "apex.h"
#include "NxApex.h"
#include "NxApexSDK.h"
#include "NxModuleIofx.h"
#include "NxModuleEmitter.h"
#include "NxModuleParticleIos.h"
#include "NxParamUtils.h"

#include "NxApexEmitterAsset.h"
#include "NxApexEmitterActor.h"
#include "NxParticleIosAsset.h"
#include "NxIofxAsset.h"
#include <NxApexRenderVolume.h>


#include "Camera.h"
#include <d3d11.h>
#include <d3dx11.h>
#include "Renderable.h"

//#include "ZeusRenderResources.h"

#include <vector>
using namespace std;
using namespace physx;

class ApexParticles : public Renderable
{
public:
    ApexParticles();
    ~ApexParticles();

	void InitPipeline();
    void CreateEmitter(NxApexSDK* gApexSDK, NxApexScene* gApexScene,
						ID3D11DeviceContext *devcon, 	ID3D11Device *dev,
					  physx::apex::NxUserRenderer* renderer, NxModuleIofx* iofxModule);

	virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);

private:
	ID3D11Device *mDev;
	ID3D11DeviceContext *mDevcon;

	ID3D11InputLayout   *mLayout;           // the pointer to the input layout
    ID3D11VertexShader  *mVS;               // the pointer to the vertex shader
    ID3D11GeometryShader  *mGS;             // the pointer to the vertex shader
    ID3D11PixelShader   *mPS;               // the pointer to the pixel shader
	ID3D11Buffer*		mConstBuffer;
	ID3D11ShaderResourceView *spriteTexture;

    physx::apex::NxApexRenderVolume*  mRenderVolume;

    NxModuleIofx* mIofxModule;
		
	apex::NxUserRenderer*      gRenderer;
};

#endif