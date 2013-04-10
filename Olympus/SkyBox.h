#pragma once

#ifndef SKYBOX_H
#define SKYBOX_H

#include "d3dUtil.h"
#include "xnamath.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "Vertices.h"
#include "Renderable.h"

class SkyBox : public Renderable
{
public:
	SkyBox();
	SkyBox(ID3D11DeviceContext *devcon, ID3D11Device *dev, GeometryGenerator *geoGen);

	void UpdateSkyBox(Camera *cam);
	void CreateGeometry(GeometryGenerator *geoGen);
	void SetupBuffer();
	virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);
	//XMMatrix skyTrans;
	ID3D11Buffer *skyBoxVertBuffer;               
	ID3D11Buffer *skyBoxIndBuffer;
	UINT indices[36];
	PosNormalTexTan vertices[24];
	ID3D11Buffer* mConstBuffer;

	ID3D11DeviceContext *mDevcon;
	ID3D11Device *mDev;
	ID3D11ShaderResourceView* mCubeMap;
};

#endif