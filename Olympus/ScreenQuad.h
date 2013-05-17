#pragma once

#ifndef ScreenQuad_H
#define ScreenQuad_H

#include "d3dUtil.h"
#include "xnamath.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "Vertices.h"
#include "Renderable.h"
#include "ConstBuffers.h"

class ScreenQuad : public Renderable
{
public:
	ScreenQuad();
	ScreenQuad(ID3D11DeviceContext *devcon, ID3D11Device *dev, GeometryGenerator *geoGen);

	void UpdateScreenQuad(Camera *cam);
	void CreateGeometry(GeometryGenerator *geoGen);
	void SetupBuffer();
	void SetupPipeline();
	void SetupRenderTarget();
	void SetupRenderTarget(int width, int height);
	virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);
	virtual void RecompileShader();
	
	ID3D11Buffer *ScreenQuadVertBuffer;               
	ID3D11Buffer *ScreenQuadIndBuffer;

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

	struct PostPBuff *cb;

};

#endif