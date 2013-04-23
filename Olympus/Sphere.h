#pragma once

#ifndef Sphere_H
#define Sphere_H

#include "d3dUtil.h"
#include "xnamath.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "Vertices.h"
#include "Renderable.h"
#include "apex.h"

struct cbuffs
{
	D3DXMATRIX viewInvProj;
	D3DXMATRIX viewPrevProj;

	float nearZ;
	float farZ;
	float padding;
	float pad;
};

class Sphere : public Renderable
{
public:
	Sphere();
	Sphere(ID3D11DeviceContext *devcon, ID3D11Device *dev, GeometryGenerator *geoGen, Apex* apex, int radius, int slices, int stacks);

	void UpdateSphere(Camera *cam);
	void CreateGeometry(GeometryGenerator *geoGen);
	void SetupBuffer();
	void SetupPipeline();
	void SetupRenderTarget();
	void getShit(ID3D11ShaderResourceView* mDynamicCubeMapSRVSphere);
	virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);
	
	ID3D11Buffer *SphereVertBuffer;               
	ID3D11Buffer *SphereIndBuffer;

	vector<UINT> indices;
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

	ID3D11ShaderResourceView* mDynamicCubeMap;

	struct cbuffs *cb;

	int radius;
	int slices;
	int stacks;
};

#endif