#pragma once

#ifndef BOX_H
#define BOX_H

#include "d3dUtil.h"
#include "xnamath.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "ScreenQuad.h"
#include "Vertices.h"
#include "Renderable.h"
#include "apex.h"
#include "ConstBuffers.h"

class Box : public Renderable
{
public:
    Box();
    Box(ID3D11DeviceContext *devcon, ID3D11Device *dev, Apex* apex, float length, float width, float height);

    void UpdateBox(Camera *cam);
    void CreateGeometry(GeometryGenerator *geoGen);
    void SetupBuffer();
    void SetupPipeline();
    
    virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);
    virtual void RecompileShader();
    virtual void Update();

	void AddInstance(float x, float y, float z);

    ID3D11Buffer *BoxVertBuffer;               
    ID3D11Buffer *BoxIndBuffer;
    ID3D11Buffer *envCBuffer;

    vector<UINT> indices;
    vector<PosNormalTexTan> vertices;

	vector<XMFLOAT4X4> mWorldMats;
    ID3D11Buffer* mConstBuffer;

    ID3D11DeviceContext *mDevcon;
    ID3D11Device *mDev;

    Apex *mApex;
    PxRigidStatic* BoxActor;

    ID3D11InputLayout   *mLayout;           // the pointer to the input layout
    ID3D11VertexShader  *mVS;               // the pointer to the vertex shader
    ID3D11PixelShader   *mPS;               // the pointer to the pixel shader

    ID3D11ShaderResourceView*   mTexture;
    ID3D11ShaderResourceView*   mNmap;
    
    

    struct cbuffs *cb;

    float mWidth, mHeight, mLength;
    float mX;
    float mY;
    float mZ;
};

#endif