#pragma once

#ifndef GroundPlane_H
#define GroundPlane_H

#include "d3dUtil.h"
#include "xnamath.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "Vertices.h"
#include "Renderable.h"



class GroundPlane : public Renderable
{
public:
	GroundPlane();
	GroundPlane(ID3D11DeviceContext *devcon, ID3D11Device *dev, GeometryGenerator *geoGen, int planeSize, int increment);

	void UpdateGroundPlane(Camera *cam);
	void CreateGeometry(GeometryGenerator *geoGen);
	void SetupBuffer();
	void SetupPipeline();
	void SetupRenderTarget();
	virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);
	
	struct PostPBuff
{
	XMFLOAT4X4 viewInvProj;
	XMFLOAT4X4 viewPrevProj;

	float nearZ;
	float farZ;
	float padding;
	float pad;
};


	ID3D11Buffer *GroundPlaneVertBuffer;               
	ID3D11Buffer *GroundPlaneIndBuffer;

	//UINT indices[6];
	vector<UINT> indices;
	//PosNormalTexTan vertices[4];
	vector<PosNormalTexTan> vertices;

	ID3D11Buffer* mConstBuffer;

	ID3D11DeviceContext *mDevcon;
	ID3D11Device *mDev;

	ID3D11InputLayout   *mLayout;           // the pointer to the input layout
    ID3D11VertexShader  *mVS;               // the pointer to the vertex shader
    ID3D11PixelShader   *mPS;               // the pointer to the pixel shader

	ID3D11Texture2D* mTargetTexture;
	ID3D11RenderTargetView* mTargetView;
	ID3D11ShaderResourceView* mShaderResourceView;

	struct PostPBuff *cb;

	int size;
	int inc;

};

#endif