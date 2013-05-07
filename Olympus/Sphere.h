#pragma once

#ifndef Sphere_H
#define Sphere_H

#include "d3dUtil.h"
#include "xnamath.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "ScreenQuad.h"
#include "Vertices.h"
#include "Renderable.h"
#include "apex.h"
#include "ConstBuffers.h"

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
	void MoveTo(float x, float y, float z);

	void SetupReflective(vector<Renderable*> *renderables, Renderable *skyBox,
						ScreenQuad *screenQuad, ID3D11DepthStencilView *zbuff,
						D3D11_VIEWPORT *screenViewport);
	void BuildCubeFaceCamera(float x, float y, float z);
	void BuildDynamicCubeMapViewsSphere();
	bool reflective;
	void DynamicCubeMapRender(ID3D11Buffer *sceneBuff, int renderType, Camera mCubeMapCamera);

	virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);
	virtual void RecompileShader();
	void IsItReflective(bool isReflective);

	ID3D11Buffer *SphereVertBuffer;               
	ID3D11Buffer *SphereIndBuffer;
	ID3D11Buffer *envCBuffer;

	vector<UINT> indices;
	vector<PosNormalTexTan> vertices;
	vector<Renderable*> *mRenderables;
	Renderable *mSkyBox;
	ScreenQuad *mScreen;
	ID3D11DepthStencilView *mZbuffer;


	ID3D11Buffer* mConstBuffer;

	ID3D11DeviceContext *mDevcon;
	ID3D11Device *mDev;

	Apex *mApex;
	PxRigidStatic* sphereActor;

	ID3D11InputLayout   *mLayout;           // the pointer to the input layout
    ID3D11VertexShader  *mVS;               // the pointer to the vertex shader
    ID3D11PixelShader   *mPS;               // the pointer to the pixel shader

	ID3D11Texture2D* mTargetTexture;
	ID3D11RenderTargetView* mTargetView;
	ID3D11ShaderResourceView* mShaderResourceView;

	Camera mCubeMapCamera[6];
	static const int CubeMapSizeSphere = 512;
	ID3D11DepthStencilView* mDynamicCubeMapDSVSphere;
	ID3D11RenderTargetView* mDynamicCubeMapRTVSphere[6];
	ID3D11ShaderResourceView* mDynamicCubeMapSRVSphere;
	D3D11_VIEWPORT mCubeMapViewport;

	D3D11_VIEWPORT *mScreenViewport;
	
	SceneBuff sphereBuff;

	struct cbuffs *cb;

	int radius;
	int slices;
	int stacks;

	float mX;
	float mY;
	float mZ;
};

#endif