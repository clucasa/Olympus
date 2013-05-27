#include "AssetManager.h"

AssetManager::AssetManager(ID3D11Device *dev, ID3D11DeviceContext *devcon) : 
    mDev(dev), mDevcon(devcon)
{

}

AssetManager::~AssetManager()
{

}

ID3D11ShaderResourceView* AssetManager::RequestTexture(string filename)
{
    Asset* asset = FindAsset(filename, AssetType::Texture);
    if(asset)
    {
        if(asset->type != AssetType::Texture)
            return NULL;
        return asset->texture;
    }
    else
    {
        asset = new Asset();
        asset->name = filename;
        asset->type = AssetType::Texture;
        HRESULT hr = D3DX11CreateShaderResourceViewFromFile( mDev, filename.c_str(), NULL, NULL, &asset->texture, NULL );
        if(FAILED(hr))
            return NULL;
        else
        {
            mTextureAssets.push_back(asset);
            return asset->texture;
        }
    }
}

void AssetManager::RequestShader(string filename)
{
    Asset* asset = FindAsset(filename, AssetType::Shader);
    if(asset)
    {
        if(asset->type != AssetType::Shader)
            return;
        //return asset->texture;
    }
    else
    {
        asset = new Asset();
        asset->name = filename;
        asset->type = AssetType::Shader;

        ID3D10Blob* pErrorBlob = NULL;
        LPVOID pError = NULL;
        char* errorStr = NULL;
        // load and compile the two shaders
        ID3D10Blob *VS, *PS;
        //D3DX11CompileFromFile(, 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, &pErrorBlob, 0);
        //if(pErrorBlob)
        //{
        //    pError = pErrorBlob->GetBufferPointer();
        //    errorStr = (char*)pError;
        //    MessageBox(0,errorStr,0,0);
        //   /* __asm {
        //        INT 3
        //    }*/
        //    //return;
        //}

        //HRESULT hr = D3DX11CreateShaderResourceViewFromFile( mDev, filename.c_str(), NULL, NULL, &asset->texture, NULL );
        //if(FAILED(hr))
        //	return NULL;
        //else
        {
            mShaderAssets.push_back(asset);
            //return asset->texture;
        }
    }
}

vector<vector<Vertex>> AssetManager::RequestModel(string filename, int &numMesh )
{
    Asset* asset = FindAsset(filename, AssetType::Model);

    if(asset)
    {
        if(asset->type != AssetType::Model)
            return *(new vector<vector<Vertex>>);
        numMesh = asset->numMeshes;
        return asset->vertexes;
    }
    else
    {
        asset = new Asset();
        asset->name = filename;
        asset->type = AssetType::Model;
        asset->numMeshes = Import( (char *)filename.c_str(), &asset->vertexes );
        if(asset->numMeshes == 0)
            return *(new vector<vector<Vertex>>);
        else
        {
            mModelAssets.push_back(asset);
            numMesh = asset->numMeshes;
            return asset->vertexes;
        }
    }
}

Asset* AssetManager::FindAsset(string filename, AssetType type)
{
    switch(type)
    {
    case AssetType::Texture:
        for(int i = 0; i < mTextureAssets.size(); i++)
        {
            if( mTextureAssets[i]->name.compare(filename) == 0 )
            {
                return mTextureAssets[i];
            }
        }
        break;

    case AssetType::Shader:
        for(int i = 0; i < mShaderAssets.size(); i++)
        {
            if( mShaderAssets[i]->name.compare(filename) == 0 )
            {
                return mShaderAssets[i];
            }
        }
        break;

    case AssetType::Model:
        for(int i = 0; i < mModelAssets.size(); i++)
        {
            if( mModelAssets[i]->name.compare(filename) == 0 )
            {
                return mModelAssets[i];
            }
        }
        break;
    };

    return NULL;
}