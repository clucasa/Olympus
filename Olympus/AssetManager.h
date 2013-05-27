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
    Shader,
    Model
};

struct Asset
{
    string name;
    AssetType type;

    //if texture asset
    ID3D11ShaderResourceView* texture;

    //if shader asset
    ID3D11PixelShader* pixelShader;
    ID3D11VertexShader* vertexShader;

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

    void RequestShader(string filename);
    vector<vector<Vertex>> RequestModel(string filename, int &numMesh);

    Asset* FindAsset(string filename, AssetType type);

    ID3D11DeviceContext *mDevcon;
    ID3D11Device *mDev;

    vector<Asset*> mTextureAssets;
    vector<Asset*> mShaderAssets;
    vector<Asset*> mModelAssets;
};

#endif