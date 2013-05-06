#pragma once

#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <vector>
#include "apex.h"
#include "Renderable.h"
#include "Camera.h"
#include "SkyBox.h"
#include "Scene.h"
#include "ScreenQuad.h"
#include "Object.h"
#include "OnScreen.h"
#include "FontSheet.h"
#include "GroundPlane.h"
#include "Sphere.h"
#include "Projectile.h"
#include "GameTimer.h"
#include "ConstBuffers.h"
#include "LightHelper.h"
using namespace std;

enum renderTargets
{
	backbuffer,
	postprocess,
	environment,
    depth
};


namespace Colors
{
	XMGLOBALCONST XMVECTORF32 White     = {1.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Black     = {0.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Red       = {1.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Green     = {0.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Blue      = {0.0f, 0.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Yellow    = {1.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Cyan      = {0.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Magenta   = {1.0f, 0.0f, 1.0f, 1.0f};

	XMGLOBALCONST XMVECTORF32 Silver    = {0.75f, 0.75f, 0.75f, 1.0f};
	XMGLOBALCONST XMVECTORF32 LightSteelBlue = {0.69f, 0.77f, 0.87f, 1.0f};
}

class RenderManager
{
public:
	
	int fps;
	float mspf;
	int SCREEN_WIDTH;
	int SCREEN_HEIGHT;


	RenderManager(ID3D11DeviceContext *devcon, 
				  ID3D11Device *dev, 
				  IDXGISwapChain *swapchain,
				  Apex *apex,
				  Camera *cam,
				  D3D11_VIEWPORT *viewport);

	void fpsCalc(GameTimer mTimer);
	void GetScreenParams(int mClientWidth, int mClientHeight);
	void Update(float dt);

	void Render(int renderType);
	void Render();
	void RenderToTarget(enum renderTargets);

	//DEBUG
	void RenderManager::SetPosition(float x, float y, float z);
	void RenderManager::SetEmit(bool on);

	void RecompShaders();
	//void InitObjects();

	IDXGISwapChain *mSwapchain;             // the pointer to the swap chain interface
	ID3D11Device *mDev;                     // the pointer to our Direct3D device interface
	ID3D11DeviceContext *mDevcon; 
	D3D11_VIEWPORT *mViewport;

	ID3D11DepthStencilView *mZbuffer;       // the pointer to our depth buffer
    ID3D11DepthStencilView *mZbuffer2;       // the pointer to our depth buffer
	ID3D11ShaderResourceView* mDepthShaderResourceView;
	ID3D11Texture2D* mDepthTargetTexture;

	ID3D11RenderTargetView* mPostProcessRTV;
	ID3D11RenderTargetView* mEnvironmentRTV;
	ID3D11RenderTargetView *mBackbuffer;    // the pointer to our back buffer

	ID3D11BlendState* mBlendState;   // Our blend state
	ID3D11SamplerState *mSampState;

	Camera *mCam;
	Camera *mScreenCam;
	GroundPlane *mGrid;
	Sphere *mSphere;
	Sphere *mSphereMove;
	Sphere *sphere2;
	
	ID3D11Buffer *sceneCBuffer;
	ID3D11Buffer *dirLightCBuffer;
	ID3D11Buffer *pointLightCBuffer;

	SkyBox *mSkyBox;
	ScreenQuad *mScreen;
	Projectile *projectile;

	ApexParticles* particles;
	ApexParticles* emitter;
	ApexParticles* torch1;
	ApexParticles* torch2;
    ApexCloth* mCloth;
	
	vector<Renderable*> renderables;

	SceneBuff sceneBuff;

	Apex                           *mApex;
	physx::apex::NxUserRenderer*    gRenderer;

	DirectionalLight mDirLight[2];
	PointLight		 mPointLight[2];

	bool emitterOn;

	FontSheet mFont;
	OnScreen mText;
	POINT textPos;
	POINT hairPos;
	POINT posPos;
	string sText;

		//Shadow Varibles
	D3D11_VIEWPORT mShadowPort;
ID3D11Texture2D* pShadowMap;
ID3D11DepthStencilView* pShadowMapDepthView;
ID3D11ShaderResourceView* pShadowMapSRView;
Camera *mShadowCam;
ID3D11Buffer *shadowCBuffer;
ShadowBuff shadowBuff;
};


#endif
