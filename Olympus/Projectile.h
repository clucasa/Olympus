#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <d3d11.h>
#include <d3dx11.h>
#include "GeometryGenerator.h"
#include "Vertices.h"
#include "Renderable.h"
#include "ConstBuffers.h"
#include "apex.h"
#include "struct.h"
#include <vector>

using namespace std;

//#define MAXBOXES 32

class Projectile : public Renderable
{
public:
    Projectile(ID3D11Device* dev, ID3D11DeviceContext* devcon, Apex* apex, int maxBoxes );
    ~Projectile();

    void Fire(Camera *mCam, float speed, vector<ApexCloth*> mCloths);
    
    virtual void Update();
    virtual void Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType);
    virtual void RecompileShader();
    virtual void Depth();

    void Clear();

    ID3D11Device *mDev;                     // the pointer to our Direct3D device interface
    ID3D11DeviceContext *mDevcon;           // the pointer to our Direct3D device context

    ID3D11InputLayout*			objLayout;
    vector<UINT> indices;
    vector<PosNormalTexTan> vertices;
    ID3D11Buffer*				vertexBuffer;
    ID3D11Buffer*				indexBuffer;

    ID3D11ShaderResourceView*   mTexture;
    ID3D11ShaderResourceView*   mNmap;
    
    ID3D11VertexShader			*opVS;			// the pointer to the vertex shader
    ID3D11PixelShader			*opPS;          // the pointer to the vertex shader

    vector<vector<physx::apex::NxClothingSphere*>> spheres;
    
    ID3D11Buffer *worldCBuffer;
    vector<XMFLOAT4X4> mWorldMats;

private:
    void						SetupSphereMesh();
    PxMaterial*					blockMaterial;
    vector<PxRigidActor*> boxes;
    int numBoxes;
    int curBox;
    int mMaxBoxes;
    Apex* mApex;
};

#endif