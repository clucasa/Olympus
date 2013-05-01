#pragma once

#ifndef CONSTBUFFERS_H
#define CONSTBUFFERS_H

#include "LightHelper.h"

struct SceneBuff
{
	XMFLOAT4X4 viewProj;
	XMFLOAT3   camPos;
	float	   phong;
	float	   normalMap;
	float	   textures;
	float      ambientOn;
	float	   diffuseOn;
	float      specularOn;
	float      pLightOn;
	float      dirLightOn;
	float	   padding;
};

struct EnvironBuff
{
    XMFLOAT4X4 ViewProj;
	XMFLOAT3 cameraPos;
	XMFLOAT3 eyePos;
	XMFLOAT2 padding;
};

struct cbuffs
{
	XMFLOAT4X4 matWorld;
	XMFLOAT4X4 matWorldInvTrans;
	Material material;
};

struct PostPBuff
{
	D3DXMATRIX viewInvProj;
	D3DXMATRIX viewPrevProj;

	float nearZ;
	float farZ;
	float lum;
	float gam;
	float depthOfField;
	float dofRange;

	XMFLOAT2 padding;
};
#endif