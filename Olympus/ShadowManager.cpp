#include "ShadowManager.h"


ShadowManager::ShadowManager(ID3D11Device *dev, ID3D11DeviceContext *devcon, int width, int height, int type) :
mDev(dev), mDevcon(devcon), mHeight(height), mWidth(width), viewType(type)
{
	//mType = DirectionalLight;

	if( type == 0 )
		mType = DirectionalLight;
	else
		mType = PointLight;

	CreateViewPort();

	DirectionalLightShadow();

	CreateConstantBuffer();

}

ShadowManager::ShadowManager(ID3D11Device *dev, ID3D11DeviceContext *devcon, int width, int height, int type, float projWidth, float projHeight) :
mDev(dev), mDevcon(devcon), mHeight(height), mWidth(width), viewType(type), mProjWidth(projWidth), mProjHeight(projHeight)
{
	// This is for the cascade shadow map
	viewType = 8;

	mType = Cascade;

	CreateViewPort();

	DirectionalLightShadow();

	CreateConstantBuffer();
}


void ShadowManager::DirectionalLightShadow()
{

	mShadowCam = new Camera();

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = mShadowPort.Width;
	descDepth.Height = mShadowPort.Height;
	descDepth.ArraySize = 1;
	descDepth.MipLevels = 1;
	descDepth.SampleDesc.Count = 1;
	descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;  

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV2;
	ZeroMemory(&descDSV2, sizeof(descDSV2));
	descDSV2.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV2.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV2.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = descDepth.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	HRESULT hr;
	hr = mDev->CreateTexture2D( &descDepth, NULL, &pShadowMap );
	if( FAILED(hr) )  
		return ;
	hr = mDev->CreateDepthStencilView( pShadowMap, &descDSV2, &pShadowMapDepthView );
	if( FAILED(hr) )  
		return;
	hr = mDev->CreateShaderResourceView( pShadowMap, &srvDesc, &pShadowMapSRView );
	if( FAILED(hr) )  
		return;
}

void ShadowManager::CreateViewPort()
{
	ZeroMemory(&mShadowPort, sizeof(D3D11_VIEWPORT));  // ALWAYS ZERO MEMORY
	mShadowPort.TopLeftX = 0;
	mShadowPort.TopLeftY = 0;
	mShadowPort.Width = mWidth;
	mShadowPort.Height = mHeight;
	mShadowPort.MaxDepth = 1;
	mShadowPort.MinDepth = 0;
}

void ShadowManager::SetViewPort()
{
	mDevcon->RSSetViewports(1, &mShadowPort);
}

void ShadowManager::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ShadowBuff);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	mDev->CreateBuffer(&bd, NULL, &shadowCBuffer);
}


void ShadowManager::SetConstantBuffer()
{
	ClearBackBuffer();
	mDevcon->RSSetViewports(1, &mShadowPort);
	XMFLOAT3 ShadPos;
	if( mType == Cascade )
	{
		mShadowCam->SetLensOrtho( -mProjWidth, mProjWidth, -mProjHeight, mProjHeight, 0.0, 1000);
		ShadPos = XMFLOAT3( -lightPos.x,// + mCam->GetPosition().x, 
			-lightPos.y,// + mCam->GetPosition().y,
			-lightPos.z );// + mCam->GetPosition().z ); 
		mShadowCam->LookAt( ShadPos, XMFLOAT3( 0,0,0 ), XMFLOAT3(0,1,0));

	}
	else if( mType == DirectionalLight )
	{
		mShadowCam->SetLensOrtho( -150.0, 150.0, -150.0, 150.0, 1.0, 1000);
		ShadPos = XMFLOAT3( -lightPos.x,// + mCam->GetPosition().x, 
			-lightPos.y,// + mCam->GetPosition().y,
			-lightPos.z );// + mCam->GetPosition().z ); 
		mShadowCam->LookAt( ShadPos, XMFLOAT3( 0,0,0 ), XMFLOAT3(0,1,0));

	}
	else if( mType == PointLight )
	{
		mShadowCam->SetLens(0.5f*MathHelper::Pi, 1.0f, 1.0f, 100.0f);
		ShadPos = lightPos;
		switch( viewType )
		{
		case 1:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x + 1.0,lightPos.y,lightPos.z ), XMFLOAT3(0,1,0));
			break;
		case 2:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x-1.0,lightPos.y,lightPos.z ), XMFLOAT3(0,1,0));
			break;
		case 3:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x,lightPos.y+1.0,lightPos.z ), XMFLOAT3(1,0,0));
			break;
		case 4:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x,lightPos.y-1.0,lightPos.z ), XMFLOAT3(-1,0,0));
			break;
		case 5:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x,lightPos.y,lightPos.z+1.0 ), XMFLOAT3(0,1,0));
			break;
		case 6:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x,lightPos.y,lightPos.z-1.0 ), XMFLOAT3(0,1,0));
			break;
		}

	}
	mShadowCam->SetPosition(ShadPos);
	mShadowCam->UpdateViewMatrix();
}


void ShadowManager::SetConstantBuffer(Camera* mCam)
{
	ClearBackBuffer();
	mDevcon->RSSetViewports(1, &mShadowPort);
	XMFLOAT3 ShadPos;
	if( mType == Cascade )
	{
		mShadowCam->SetLensOrtho( -mProjWidth, mProjWidth, -mProjHeight, mProjHeight, 1.0, 3000);
		ShadPos = XMFLOAT3( 1500.f*(-lightPos.x) + mCam->GetPosition().x, 
			1500.f*(-lightPos.y) + mCam->GetPosition().y,
			1500.f*(-lightPos.z) + mCam->GetPosition().z );
	
		mShadowCam->LookAt( ShadPos, mCam->GetPosition(), XMFLOAT3(0,1,0));
	}
	else if( mType == DirectionalLight )
	{
		mShadowCam->SetLensOrtho( -150.0, 150.0, -150.0, 150.0, 1.0, 1000);
		ShadPos = XMFLOAT3( -lightPos.x + mCam->GetPosition().x, 
			-lightPos.y + mCam->GetPosition().y,
			-lightPos.z + mCam->GetPosition().z ); 
		mShadowCam->LookAt( ShadPos, mCam->GetPosition(), XMFLOAT3(0,1,0));

	}
	else if( mType == PointLight )
	{
		mShadowCam->SetLens(0.5f*MathHelper::Pi, 1.0f, 1.0f, 100.0f);
		lightPos = XMFLOAT3( mCam->GetPosition().x,mCam->GetPosition().y-1.5,mCam->GetPosition().z );//XMFLOAT3( 7.7, 0, 66 );//mCam->GetPosition();
		ShadPos = lightPos;
		//ShadPos = XMFLOAT3( mCam->GetPosition().x,mCam->GetPosition().y + 3.0,mCam->GetPosition().z);
		switch( viewType )
		{
		case 1:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x+1.0,lightPos.y,lightPos.z ), XMFLOAT3(0,1,0));
			break;
		case 2:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x-1.0,lightPos.y,lightPos.z ), XMFLOAT3(0,1,0));
			break;
		case 3:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x,lightPos.y+1.0,lightPos.z ), XMFLOAT3(1,0,0));
			break;
		case 4:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x,lightPos.y-1.0,lightPos.z ), XMFLOAT3(-1,0,0));
			break;
		case 5:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x,lightPos.y,lightPos.z+1.0 ), XMFLOAT3(0,1,0));
			break;
		case 6:
			mShadowCam->LookAt( ShadPos, XMFLOAT3( lightPos.x,lightPos.y,lightPos.z-1.0 ), XMFLOAT3(0,1,0));
			break;
		}

	}
	mShadowCam->SetPosition(ShadPos);
	mShadowCam->UpdateViewMatrix();
}

void ShadowManager::SetSampler()
{
	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sd.MaxAnisotropy = 1;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;

	sd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	mDev->CreateSamplerState(&sd, &pSS);
	mDevcon->PSSetSamplers(5, 1, &pSS);
}

void ShadowManager::ClearBackBuffer()
{
	mDevcon->ClearDepthStencilView(pShadowMapDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

int ShadowManager::SetSRV(int loc)
{
	mDevcon->PSSetShaderResources(loc, 1, &pShadowMapSRView);
	return 1;
}