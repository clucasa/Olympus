#pragma once

#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <vector>
#include "Renderable.h"
#include "Camera.h"
#include "SkyBox.h"

using namespace std;

enum renderTargets
	{
		backbuffer,
		postprocess,
		environment
	};

class RenderManager
{
public:
	

	RenderManager(ID3D11DeviceContext *devcon, 
				  ID3D11Device *dev, 
				  IDXGISwapChain *swapchain,
				  Camera *cam);

	void Render(int renderType);
	void Render();
	void RenderToTarget(enum renderTargets);

	IDXGISwapChain *mSwapchain;             // the pointer to the swap chain interface
	ID3D11Device *mDev;                     // the pointer to our Direct3D device interface
	ID3D11DeviceContext *mDevcon; 

	ID3D11RenderTargetView* mPostProcessRTV;
	ID3D11RenderTargetView* mEnvironmentRTV;
	ID3D11RenderTargetView *mBackbuffer;    // the pointer to our back buffer

	Camera *mCam;

	ID3D11Buffer *sceneCBuffer;

	SkyBox *mSkyBox;

	vector<Renderable*> renderables;

	

};

#endif