#pragma once

#ifndef OBJECT_H
#define OBJECT_H

#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <vector>
#include <string>
#include "importer.h"
#include "Renderable.h"
#include "apex.h"
#include "LightHelper.h"
#include "ConstBuffers.h"

using namespace std;

#ifndef OBJECT_INFO
#define OBJECT_INFO
struct ObjectInfo
{
	float x,y,z;
	float sx,sy,sz;
	float rx, ry, rz;
};
#endif

class Object : public Renderable
{
public:
	int numMeshes;
	int alpha;

	Apex* mApex;

	ID3D11InputLayout* objLayout;
	//ID3D11DeviceContext * devcon1;
	vector<Vertex>		vertices;
	vector<vector<Vertex>> vertexes;

	vector<ID3D11Buffer*>		vertexBuffer;
	vector<ID3D11ShaderResourceView*> texArray;
	vector<ID3D11ShaderResourceView*> NormArray;

	ID3D10Blob *oVS, *oPS;
	ID3D11VertexShader *opVS;               // the pointer to the vertex shader
	ID3D11PixelShader *opPS;               // the pointer to the vertex shader

	
	ID3D11Buffer *worldCBuffer;
	XMFLOAT4X4 mWorldMat;

	vector<XMFLOAT4X4> mWorldMats;
	vector<Material> materials;

	cbuffs cb;

	Object();
	void objLoad( char* filename, vector<LPCSTR > *textures, vector<LPCSTR > *NormTextures, ID3D11Device* devv, ID3D11DeviceContext *devcon, Apex* apex );
	void renderO(ID3D11DeviceContext *devcon);
	virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);
	virtual void RecompileShader();

	void AddInstance(ObjectInfo info);

private:
	HRESULT hr1;
	vector<Object> objList;
	int objDex;

	ID3D11Device *dev1;                     // the pointer to our Direct3D device interface
	ID3D11DeviceContext *devcon1;           // the pointer to our Direct3D device context

	ID3D11Buffer *pVBuffer1;                // the pointer to the vertex buffer
	ID3D11Buffer *pIBuffer1;
	ID3D11Buffer *pCBuffer1;                // the pointer to the constant buffer

	ID3D11ShaderResourceView*   g_pTextureRV1;

	ID3D11ShaderResourceView**  g_pTexArray1;
	ID3D11ShaderResourceView**  g_pNormArray1;

};
#endif