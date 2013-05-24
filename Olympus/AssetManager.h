#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <vector>
#include <string>
#include <d3d11.h>
#include <d3dx11.h>

using namespace std;

struct Asset
{
    string name;
    ID3D11ShaderResourceView* texture;

};

class AssetManager
{
public:
    AssetManager(ID3D11Device *dev, ID3D11DeviceContext *devcon);
    ~AssetManager();

    ID3D11ShaderResourceView* RequestTexture(string filename);

    void RequestShader(string filename);
    void RequestVertexBuffer(string filename);

    Asset* FindAsset(string filename);

    ID3D11DeviceContext *mDevcon;
    ID3D11Device *mDev;

    vector<Asset*> mAssets;
};

#endif