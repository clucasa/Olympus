#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <d3d11.h>
#include <d3dx11.h>
#include "Renderable.h"
#include "apex.h"
#include "struct.h"
#include <vector>

using namespace std;

#define MAXBOXES 10000

class Projectile : public Renderable
{
public:
	Projectile(ID3D11Device* dev, ID3D11DeviceContext* devcon, Apex* apex );
	~Projectile();

	void Fire(Camera *mCam, float speed);
	
	void Update();
	virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);
	virtual void RecompileShader();

	ID3D11Device *mDev;                     // the pointer to our Direct3D device interface
	ID3D11DeviceContext *mDevcon;           // the pointer to our Direct3D device context

	ID3D11InputLayout*			objLayout;
	vector<Vertex>				vertices;
	ID3D11Buffer*				vertexBuffer;

    ID3D11ShaderResourceView*   mTexture;
    ID3D11ShaderResourceView*   mNmap;
	
	ID3D11VertexShader			*opVS;			// the pointer to the vertex shader
	ID3D11PixelShader			*opPS;          // the pointer to the vertex shader

	
	ID3D11Buffer *worldCBuffer;
	vector<XMFLOAT4X4> mWorldMats;

private:
	void						SetupBoxMesh();
	PxMaterial*					blockMaterial;
	vector<PxRigidActor*> boxes;
	int numBoxes;
	Apex* mApex;
};

#endif