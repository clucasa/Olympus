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
#include "ScreenQuad.h"
#include "Object.h"
#include "Lighting.h"

using namespace std;

enum renderTargets
	{
		backbuffer,
		postprocess,
		environment
	};

struct SceneBuff
{
	XMFLOAT4X4 viewProj;
	XMFLOAT3   camPos;
	float	   pad;
};

struct DirectionalLight
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT4 Direction;
	float  SpecPower;
	XMFLOAT3 pad;
};

struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	// Packed into 4D vector: (Position, Range)
	XMFLOAT3 Position;
	float Range;

	// Packed into 4D vector: (A0, A1, A2, Pad)
	XMFLOAT3 Att;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

class RenderManager
{
public:
	

	RenderManager(ID3D11DeviceContext *devcon, 
				  ID3D11Device *dev, 
				  IDXGISwapChain *swapchain,
				  Apex *apex,
				  Camera *cam);

	void Render(int renderType);
	void Render();
	void RenderToTarget(enum renderTargets);

	//void InitObjects();

	IDXGISwapChain *mSwapchain;             // the pointer to the swap chain interface
	ID3D11Device *mDev;                     // the pointer to our Direct3D device interface
	ID3D11DeviceContext *mDevcon; 

	ID3D11DepthStencilView *mZbuffer;       // the pointer to our depth buffer
    ID3D11DepthStencilView *mZbuffer2;       // the pointer to our depth buffer
	ID3D11ShaderResourceView* mDepthShaderResourceView;
	ID3D11Texture2D* mDepthTargetTexture;

	ID3D11RenderTargetView* mPostProcessRTV;
	ID3D11RenderTargetView* mEnvironmentRTV;
	ID3D11RenderTargetView *mBackbuffer;    // the pointer to our back buffer

	ID3D11BlendState* mBlendState;   // Our blend state

	Camera *mCam;
	Camera *mScreenCam;

	ID3D11Buffer *sceneCBuffer;
	ID3D11Buffer *dirLightCBuffer;
	ID3D11Buffer *pointLightCBuffer;

	SkyBox *mSkyBox;
	ScreenQuad *mScreen;
	
	vector<Renderable*> renderables;

	SceneBuff sceneBuff;

	Apex                           *mApex;
	physx::apex::NxUserRenderer*    gRenderer;

	DirectionalLight mDirLight;
	PointLight		 mPointLight[2];
};


#endif
