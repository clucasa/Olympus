#pragma once

#ifndef SHADOWMANAGER_H
#define SHADOWMANAGER_H

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <vector>
#include "Camera.h"
#include "ConstBuffers.h"
#include "MathHelper.h"


using namespace std;

class ShadowManager
{
public:
	
	enum ShadowType
	{
		DirectionalLight,
		PointLight,
		Cascade
	};

	ShadowManager(ID3D11Device *dev, ID3D11DeviceContext *devcon, int width, int height, int type);
	ShadowManager(ID3D11Device *dev, ID3D11DeviceContext *devcon, int width, int height, int type, float projWidth, float projHeight);
	void DirectionalLightShadow();
	void PointLightShadow();
	void CascadeLightShadow();
	void CreateViewPort();
	void SetViewPort();
	void CreateConstantBuffer();
	void SetConstantBuffer();
	void SetConstantBuffer(Camera* mCam);
	void SetSampler();
	void ClearBackBuffer();
	int SetSRV(int loc);

	ShadowType mType;

	ID3D11SamplerState* pSS;

	ID3D11Device *mDev;
	ID3D11DeviceContext *mDevcon;

	int mWidth;
	int mHeight;

	//Shadow Varibles
	D3D11_VIEWPORT mShadowPort;

	ID3D11Texture2D*			pShadowMap;
	ID3D11DepthStencilView*		pShadowMapDepthView;
	ID3D11ShaderResourceView*	pShadowMapSRView;

	XMFLOAT3 lightPos;

	Camera *mShadowCam;

	ID3D11Buffer *shadowCBuffer;
	ShadowBuff shadowBuff;

	int viewType;
	float mProjWidth;
	float mProjHeight;
};

#endif
