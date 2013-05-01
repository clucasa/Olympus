#pragma once

#ifndef CONSTBUFFERS_H
#define CONSTBUFFERS_H

#include "LightHelper.h"

struct SceneBuff
{
	XMFLOAT4X4 viewProj;
	XMFLOAT3   camPos;
	float	   pad;
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

#endif