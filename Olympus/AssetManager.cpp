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
	Asset* asset = FindAsset(filename);
	if(asset)
	{
		return asset->texture;
	}
	else
	{
		asset = new Asset();
		asset->name = filename;
		HRESULT hr = D3DX11CreateShaderResourceViewFromFile( mDev, filename.c_str(), NULL, NULL, &asset->texture, NULL );
		if(FAILED(hr))
			return NULL;
		else
		{
			mAssets.push_back(asset);
			return asset->texture;
		}
	}
}

void AssetManager::RequestShader(string filename)
{

}

void AssetManager::RequestVertexBuffer(string filename)
{

}

Asset* AssetManager::FindAsset(string filename)
{
	for(int i = 0; i < mAssets.size(); i++)
	{
		if( mAssets[i]->name.compare(filename) == 0 )
		{
			return mAssets[i];
		}
	}

	return NULL;
}