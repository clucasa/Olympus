#pragma once
//***************************************************************************************
// ApexParticles.h
//
//
// 
//
//***************************************************************************************
#ifndef APEX_DESTRUCTABLE_H
#define APEX_DESTRUCTABLE_H

//#include "apex.h"
#include "NxApex.h"
#include "NxApexSDK.h"
#include "NxModuleClothing.h"
#include "NxClothingAsset.h"
#include "NxClothingActor.h"

#include "NxParamUtils.h"
#include <NxApexRenderVolume.h>


#include "Camera.h"
#include <d3d11.h>
#include <d3dx11.h>
#include "Renderable.h"

//#include "ZeusRenderResources.h"

#include <vector>
using namespace std;
using namespace physx;

class ApexDestructable : public Renderable
{
public:
    ApexDestructable();
    ~ApexDestructable();

    void InitPipeline();

    void CreateDestructable(NxApexSDK* gApexSDK, NxApexScene* gApexScene,
                        ID3D11DeviceContext *devcon, 	ID3D11Device *dev,
                      physx::apex::NxUserRenderer* renderer, const char* filename);
    void Update();
    virtual void RecompileShader();

    void SetPosition(float x, float y, float z);
    virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);

private:
    ID3D11Device *mDev;
    ID3D11DeviceContext *mDevcon;

    ID3D11InputLayout   *mLayout;           // the pointer to the input layout
    ID3D11VertexShader  *mVS;               // the pointer to the vertex shader
    ID3D11GeometryShader  *mGS;             // the pointer to the vertex shader
    ID3D11PixelShader   *mPS;               // the pointer to the pixel shader
    ID3D11Buffer*		mConstBuffer;
    ID3D11ShaderResourceView *clothTexture;


    //NxApexEmitterActor* emitterActor;
   // physx::apex::NxApexRenderVolume*  mRenderVolume;
    physx::apex::NxClothingActor* clothingActor;
            
    apex::NxUserRenderer*      gRenderer;
};

#endif