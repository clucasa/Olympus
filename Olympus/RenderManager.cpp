#include "RenderManager.h"
#include "GeometryGenerator.h"
#include "System.h"

RenderManager::RenderManager(ID3D11DeviceContext *devcon, 
							 ID3D11Device *dev, 
							 IDXGISwapChain *swapchain,
							 Apex *apex,
							 Camera *cam) :
mDevcon(devcon), mDev(dev), mSwapchain(swapchain), mCam(cam), mApex(apex)
{
	ID3D11Texture2D *pBackBuffer;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // use the back buffer address to create the render target
    dev->CreateRenderTargetView(pBackBuffer, NULL, &mBackbuffer);
    pBackBuffer->Release();

	gRenderer = new ZeusRenderer();

	GeometryGenerator *geoGen = new GeometryGenerator();
	
	mSkyBox = new SkyBox(mDevcon, mDev, geoGen);
	
	ScreenQuad *sq = new ScreenQuad(mDevcon, mDev, geoGen);



	Renderable* particles = (Renderable*)apex->CreateEmitter(gRenderer);
		
	//Special "renderable" case, do not add to the vector
	mScreen = new ScreenQuad(mDevcon, mDev, geoGen);
	//Special camera, doesn't move

	//renderables.push_back(sq);
	renderables.push_back(particles);

    vector<LPSTR> textures;
	vector<LPSTR> normalMap;

    textures.push_back( "Media/Textures/CommandoArmor_DM.dds" );
	textures.push_back( "Media/Textures/Commando_DM.dds" );
	normalMap.push_back( "Media/Textures/CommandoArmor_NM.dds" );
	normalMap.push_back( "Media/Textures/Commando_NM.dds" );

	Object* obj = new Object();
	obj->objLoad( "Media/Models/bigbadman.fbx", &textures, &normalMap, dev, devcon);

    renderables.push_back(obj);

	mScreenCam = new Camera();
	mScreenCam->UpdateViewMatrix();

	free(geoGen);

    
    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 64;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &sceneCBuffer);

	//CREATE POST PROCESS RTV
	ID3D11Texture2D* pBuffer;
	mSwapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBuffer );
	

	mDev->CreateRenderTargetView( pBuffer, NULL, &mPostProcessRTV );
	pBuffer->Release();


	// create the depth buffer texture
	HRESULT hr;

	ID3D11RasterizerState*		pState;
	D3D11_RASTERIZER_DESC		raster;
	ZeroMemory( &raster, sizeof(D3D11_RASTERIZER_DESC));

	raster.FillMode = D3D11_FILL_SOLID;
	raster.CullMode = D3D11_CULL_BACK;
	raster.FrontCounterClockwise = FALSE;
	raster.DepthBias = 0;
	raster.DepthBiasClamp = 0.0f;
	raster.SlopeScaledDepthBias = 0.0f;
	raster.DepthClipEnable = TRUE; //set for testing otherwise true
	raster.ScissorEnable = FALSE;
	raster.MultisampleEnable = FALSE;
	raster.AntialiasedLineEnable = FALSE;

	hr = dev->CreateRasterizerState (&raster, &pState);
	devcon->RSSetState( pState );

	D3D11_TEXTURE2D_DESC texd;
	ZeroMemory(&texd, sizeof(texd));

	texd.Width = SCREEN_WIDTH;
	texd.Height = SCREEN_HEIGHT;
	texd.ArraySize = 1;
	texd.MipLevels = 1;
	texd.SampleDesc.Count = 1;
	texd.Format = DXGI_FORMAT_R32_TYPELESS;
	texd.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	dev->CreateTexture2D(&texd, NULL, &mDepthTargetTexture);


	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(dsvd));

	dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;

	dev->CreateDepthStencilView(mDepthTargetTexture, &dsvd, &mZbuffer);

	texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dev->CreateDepthStencilView(mDepthTargetTexture, &dsvd, &mZbuffer2);

	
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	dev->CreateShaderResourceView(mDepthTargetTexture, &shaderResourceViewDesc, &mDepthShaderResourceView);
	
	

    // Init blending
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = TRUE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    hr = dev->CreateBlendState(&blendDesc, &mBlendState);
    if( FAILED(hr) )
        return;

    float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f}; 
    devcon->OMSetBlendState(mBlendState, blendFactor, 0xffffffff); // restore default

	//Set the directional light
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(DirectionalLight);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &dirLightCBuffer);

	mDirLight.Ambient =  XMFLOAT4(.3f, .3f, .3f, 1);
	mDirLight.Diffuse =  XMFLOAT4(.6f, .6f, .6f, 1);
	mDirLight.Direction =  XMFLOAT4(5, 0, 0, 1);
	mDirLight.Specular =  XMFLOAT4(1, 1, 1, 1);
	mDirLight.SpecPower = 10.0f;


	//Set the point light
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(PointLight) * 2;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &pointLightCBuffer);

	mPointLight[0].Ambient = XMFLOAT4(.3f, .1f, .1f, 1);
	mPointLight[0].Att     = XMFLOAT3(.4f, .4f, .4f);
	mPointLight[0].Diffuse = XMFLOAT4(.6f, .0f, .0f, 1);
	mPointLight[0].Specular = XMFLOAT4(1, 1, 1, 1);
	mPointLight[0].Range    = 2.0f;
	mPointLight[0].Position = XMFLOAT3(3, 0, 0);

	mPointLight[1].Ambient = XMFLOAT4(.3f, .1f, .1f, 1);
	mPointLight[1].Att     = XMFLOAT3(.4f, .4f, .4f);
	mPointLight[1].Diffuse = XMFLOAT4(.0f, .6f, .0f, 1);
	mPointLight[1].Specular = XMFLOAT4(1, 1, 1, 1);
	mPointLight[1].Range    = 2.0f;
	mPointLight[1].Position = XMFLOAT3(-3, 0, 0);
}


void RenderManager::Render()
{
	// clear the back buffer to a deep blue
	mDevcon->ClearDepthStencilView(mZbuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    mDevcon->ClearRenderTargetView(mBackbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));
	mDevcon->ClearRenderTargetView(mScreen->mTargetView, D3DXCOLOR(0.0f, 1.0f, 0.4f, 1.0f));
	
	XMStoreFloat4x4(&sceneBuff.viewProj, mCam->ViewProj());

	sceneBuff.camPos = mCam->GetPosition();

	mDevcon->VSSetConstantBuffers(0, 1, &sceneCBuffer);
	mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, &sceneBuff , 0, 0);

	//Skybox right now doesn't like zbuffers, so dont' set one for it
	mDevcon->OMSetRenderTargets(1, &mScreen->mTargetView, 0);
	mSkyBox->Render(sceneCBuffer, mCam, 0);


	mDevcon->OMSetRenderTargets(1, &mScreen->mTargetView/*mBackbuffer*/, mZbuffer);
	
	mDevcon->PSSetConstantBuffers(2, 1, &dirLightCBuffer);
	mDevcon->UpdateSubresource(dirLightCBuffer, 0, 0, &mDirLight, 0, 0);

	mDevcon->PSSetConstantBuffers(3, 1, &pointLightCBuffer);
	mDevcon->UpdateSubresource(pointLightCBuffer, 0, 0, &mPointLight, 0, 0);

    Render(0);

	
	//Render the screen quad last
	mDevcon->OMSetRenderTargets(1, &mBackbuffer, 0);
	
	mDevcon->PSSetShaderResources(1, 1, &mDepthShaderResourceView);

	mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, mScreenCam->ViewProj().m , 0, 0);
	mScreen->Render(sceneCBuffer, mScreenCam, 0);
}

void RenderManager::Render(int renderType)
{
	for(int i = 0; i < renderables.size() ; i++)
	{
		renderables[i]->Render(sceneCBuffer, mCam, renderType);
	}
}

void RenderManager::RenderToTarget(enum renderTargets target)
{
	switch(target)
	{
	case backbuffer:
		mDevcon->OMSetRenderTargets(0, &mBackbuffer, NULL);
		break;
	case postprocess:
		mDevcon->OMSetRenderTargets(0, &mScreen->mTargetView, mZbuffer);
		break;
	}
}
