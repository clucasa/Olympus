#pragma once

#ifndef GRID_H
#define GRID_H

#include "d3dUtil.h"
#include "xnamath.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "Vertices.h"
#include "Renderable.h"



class Grid : public Renderable
{
public:
	Grid();
	Grid(ID3D11DeviceContext *devcon, ID3D11Device *dev, GeometryGenerator *geoGen);

	void UpdateGrid(Camera *cam);
	void CreateGeometry(GeometryGenerator *geoGen);
	void SetupBuffer();
	void SetupPipeline();
	void SetupRenderTarget();
	virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);
	
	ID3D11Buffer *GridVertBuffer;               
	ID3D11Buffer *GridIndBuffer;

	UINT indices[6];
	PosNormalTexTan vertices[4];

	ID3D11Buffer* mConstBuffer;

	ID3D11DeviceContext *mDevcon;
	ID3D11Device *mDev;

	ID3D11InputLayout   *mLayout;           // the pointer to the input layout
    ID3D11VertexShader  *mVS;               // the pointer to the vertex shader
    ID3D11PixelShader   *mPS;               // the pointer to the pixel shader

	ID3D11Texture2D* mTargetTexture;
	ID3D11RenderTargetView* mTargetView;
	ID3D11ShaderResourceView* mShaderResourceView;


};

#endif