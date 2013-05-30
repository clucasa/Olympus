#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <vector>
#include <string>
#include <d3d11.h>
#include <d3dx11.h>

#include "importer.h"

using namespace std;

enum AssetType
{
    Texture,
    VShader,
    PShader,
    Model
};

struct Asset
{
    string name;
    AssetType type;

    //if texture asset
    ID3D11ShaderResourceView* texture;
    
    //if vShader
    ID3D11VertexShader* vertexShader;
    ID3D10Blob* VS;

    //if pShader asset
    ID3D11PixelShader* pixelShader;
    ID3D10Blob* PS;

    //if model asset
    vector<vector<Vertex>> vertexes;
    int numMeshes;
};

class AssetManager
{
public:
    AssetManager(ID3D11Device *dev, ID3D11DeviceContext *devcon);
    ~AssetManager();

    ID3D11ShaderResourceView* RequestTexture(string filename);

    Asset* RequestVShader(string filename);
    Asset* RequestPShader(string filename);

    vector<vector<Vertex>> RequestModel(string filename, int &numMesh);

    Asset* FindAsset(string filename, AssetType type);

    void RecompileShaders();

    ID3D11DeviceContext *mDevcon;
    ID3D11Device *mDev;

    vector<Asset*> mTextureAssets;
    vector<Asset*> mVShaderAssets;
    vector<Asset*> mPShaderAssets;
    vector<Asset*> mModelAssets;
};

#endif