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

	emitter = apex->CreateEmitter(gRenderer);

	particles = apex->CreateEmitter(gRenderer);
		
	//Special "renderable" case, do not add to the vector
	mScreen = new ScreenQuad(mDevcon, mDev, geoGen);
	//Special camera, doesn't move

	//renderables.push_back(sq);
	

 //   vector<LPSTR> textures;
	//vector<LPSTR> normalMap;

 //   textures.push_back( "Media/Textures/CommandoArmor_DM.dds" );
	//textures.push_back( "Media/Textures/Commando_DM.dds" );
	//normalMap.push_back( "Media/Textures/CommandoArmor_NM.dds" );
	//normalMap.push_back( "Media/Textures/Commando_NM.dds" );

	//Object* obj = new Object();
	//obj->objLoad( "Media/Models/bigbadman.fbx", &textures, &normalMap, dev, devcon, apex);

 //   renderables.push_back(obj);

	Scene* scene = new Scene(&renderables, dev, devcon, apex);
	
	mSphere = new Sphere(mDevcon, mDev, geoGen, apex, 3, 30, 30);
	renderables.push_back(mSphere);

	mGrid = new GroundPlane(mDevcon, mDev, geoGen, 400, 10);
	renderables.push_back(mGrid);

	HRESULT hr;

	//mFont;// = new FontSheet();
	//mText;//  = OnScreen();

	hr = mFont.Initialize(mDev, L"Times New Roman", 30.0f, FontSheet::FontStyleRegular, false);
	hr = mText.Initialize(mDev);

	sText = L"Dragon Slayer";

	// Calculate the text width.
	int textWidth = 0;
	for(UINT i = 0; i < sText.size(); ++i)
	{
		WCHAR character = sText[i];
		if(character == ' ') 
		{
			textWidth += mFont.GetSpaceWidth();
		}
		else{
			const CD3D11_RECT& r = mFont.GetCharRect(sText[i]);
			textWidth += (r.right - r.left + 1);
		}
	}

        textPos.x = (SCREEN_WIDTH - textWidth) - 2.0;
        textPos.y = 0;//SCREEN_HEIGHT;

	//hr = D3DX11CreateShaderResourceViewFromFile(dev, "Textures/WoodCrate01.dds", 0, 0, &mImageSRV, 0 );


	mScreenCam = new Camera();
    mScreenCam->SetLensOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1000.0f);
	mScreenCam->UpdateViewMatrix();

	free(geoGen);

	renderables.push_back(emitter);
	renderables.push_back(particles);
    
    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 64;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &sceneCBuffer);

	////CREATE POST PROCESS RTV
	//ID3D11Texture2D* pBuffer;
	//mSwapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBuffer );
	//

	//mDev->CreateRenderTargetView( pBuffer, NULL, &mPostProcessRTV );
	//pBuffer->Release();


	// create the depth buffer texture
	

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
    bd.ByteWidth = sizeof(DirectionalLight)*2;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &dirLightCBuffer);
	
	mDirLight[0].Ambient =		XMFLOAT4(.55f, .55f, .55f, 1);
	mDirLight[0].Diffuse =		XMFLOAT4(.4f, .4f, .4f, 1);
	//mDirLight[0].Direction =	XMFLOAT4(-0.57735f, -0.57735f, 0.57735f, 1.0);
	mDirLight[0].Direction =	XMFLOAT4(0, 0, 5.0f, 1.0);
	mDirLight[0].Specular =		XMFLOAT4(0.8f, 0.8f, 0.7f, 1);
	mDirLight[0].SpecPower =	8.0f;

	mDirLight[1].Ambient =		XMFLOAT4(.3f, .3f, .3f, 1);
	mDirLight[1].Diffuse =		XMFLOAT4(.6f, .6f, .6f, 1);
	mDirLight[1].Direction =	XMFLOAT4(10, 0, 0, 1);
	mDirLight[1].Specular =		XMFLOAT4(1, 1, 1, 1);
	mDirLight[1].SpecPower =	8.0f;


	//Set the point light
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(PointLight) * 2;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &pointLightCBuffer);

	mPointLight[0].Ambient = XMFLOAT4(.1f, .1f, .1f, 1);
	mPointLight[0].Att     = XMFLOAT3(1.0f, .05f, .0075f);
	mPointLight[0].Diffuse = XMFLOAT4(.6f, .0f, .0f, 1);
	mPointLight[0].Specular = XMFLOAT4(1, 1, 1, 1);
	mPointLight[0].Range    = 10.0f;
	mPointLight[0].Position = XMFLOAT3(0, 15, 0);

	mPointLight[1].Ambient = XMFLOAT4(.1f, .1f, .1f, 1);
	mPointLight[1].Att     = XMFLOAT3(1.0f, .05f, .0075f);
	mPointLight[1].Diffuse = XMFLOAT4(.0f, .6f, .0f, 1);
	mPointLight[1].Specular = XMFLOAT4(1, 1, 1, 1);
	mPointLight[1].Range    = 6.0f;
	mPointLight[1].Position = XMFLOAT3(0, 0, -3);

	BuildDynamicCubeMapViewsSphere();
}


void RenderManager::Render()
{
	// clear the back buffer to a deep blue
	mDevcon->ClearDepthStencilView(mZbuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    mDevcon->ClearRenderTargetView(mBackbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));
	mDevcon->ClearRenderTargetView(mScreen->mTargetView, D3DXCOLOR(0.0f, 1.0f, 0.4f, 1.0f));
	
	particles->Update();
	emitter->Update();
	mDevcon->RSSetState(0);
    ID3D11RenderTargetView* renderTargets[1];

	BuildCubeFaceCamera(-25.0f, 4.0f, 20.0f); // Sphere position
    
	cameraCount = 0;

    for(int i = cameraCount = 0; i < 6; ++i, ++cameraCount) // for mirror, just do (int i = 0; i < 1; ++i) for 1 camera mapped to mirror surface
    {
		// Clear cube map face and depth buffer.
		mDevcon->ClearRenderTargetView(mDynamicCubeMapRTVSphere[i], reinterpret_cast<const float*>(&Colors::Silver));
        mDevcon->ClearDepthStencilView(mDynamicCubeMapDSVSphere, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

        // Bind cube map face as render target.
        renderTargets[0] = mDynamicCubeMapRTVSphere[i];
        mDevcon->OMSetRenderTargets(1, renderTargets, mDynamicCubeMapDSVSphere);

		XMStoreFloat4x4(&sceneBuff.viewProj, mCubeMapCamera[cameraCount].ViewProj());
		sceneBuff.camPos = XMFLOAT3(0.5,0,0);
		mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, &sceneBuff , 0, 0);

		mDevcon->VSSetConstantBuffers(0, 1, &sceneCBuffer);
		mDevcon->PSSetConstantBuffers(0, 1, &sceneCBuffer);

        // Draw the scene with the exception of the center sphere to this cube map face
		//mSkyBox->Render(sceneCBuffer, &mCubeMapCamera[cameraCount], 0);
	
        DynamicCubeMapRender(0, mCubeMapCamera[cameraCount]);
		//mDevcon->ClearRenderTargetView(mDynamicCubeMapRTVSphere[i], reinterpret_cast<const float*>(&Colors::Silver));
        //mDevcon->ClearDepthStencilView(mDynamicCubeMapDSVSphere, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
		//DynamicCubeMapRender(0, mCubeMapCamera[cameraCount]);
		//++cameraCount;
    }
	mDevcon->GenerateMips(mDynamicCubeMapSRVSphere);
	mSphere->getShit(mDynamicCubeMapSRVSphere);//mSkyBox->mCubeMap


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

	mText.DrawString(mDevcon, mFont, sText, textPos, XMCOLOR(0xffffffff));
}

void RenderManager::Render(int renderType)
{
	for(int i = 0; i < renderables.size() ; i++)
	{
		renderables[i]->Render(sceneCBuffer, mCam, renderType);
	}
}

void RenderManager::DynamicCubeMapRender(int renderType, Camera cam)
{
	for(int i = 0; i < renderables.size() ; i++)
	{
		sceneBuff.camPos = XMFLOAT3(0.5,0,0);
		mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, &sceneBuff , 0, 0);
		mDevcon->PSSetConstantBuffers(0, 1, &sceneCBuffer);
		renderables[i]->Render(sceneCBuffer, &mCubeMapCamera[cameraCount], renderTargets::environment);
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


//DEBUG
void RenderManager::SetPosition(float x, float y, float z)
{
	particles->SetPosition(x,y,z);
}

void RenderManager::SetEmit(bool on)
{
	particles->SetEmit(on);
}

void RenderManager::RecompShaders()
{
	mScreen->RecompileShader();

	for(int i = 0; i < renderables.size() ; i++)
	{
		renderables[i]->RecompileShader();
	}
}

void RenderManager::BuildCubeFaceCamera(float x, float y, float z)
{
    // Generate the cube map about the given position.
    XMFLOAT3 center(x, y, z);
    XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);

    // Look along each coordinate axis.
    XMFLOAT3 targets[6] = 
    {
        XMFLOAT3(x+1.0f, y, z), // +X
        XMFLOAT3(x-1.0f, y, z), // -X
        XMFLOAT3(x, y+1.0f, z), // +Y
        XMFLOAT3(x, y-1.0f, z), // -Y
        XMFLOAT3(x, y, z+1.0f), // +Z
        XMFLOAT3(x, y, z-1.0f)  // -Z
    };

    // Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we are looking down +Y or -Y, so we need a different "up" vector.
    XMFLOAT3 ups[6] = 
    {
        XMFLOAT3(0.0f, 1.0f, 0.0f),  // +X
        XMFLOAT3(0.0f, 1.0f, 0.0f),  // -X
        XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
        XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
        XMFLOAT3(0.0f, 1.0f, 0.0f),	 // +Z
        XMFLOAT3(0.0f, 1.0f, 0.0f)	 // -Z
    };

    for(int i = 0; i < 6; ++i)
    {
        mCubeMapCamera[i].LookAt(center, targets[i], ups[i]);
        mCubeMapCamera[i].SetLens(0.5f*XM_PI, 1.0f, 0.1f, 500.0f);
        mCubeMapCamera[i].UpdateViewMatrix();
    }
}


void RenderManager::BuildDynamicCubeMapViewsSphere()
{
    // Cubemap is a special texture array with 6 elements.
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = CubeMapSizeSphere;
    texDesc.Height = CubeMapSizeSphere;
    texDesc.MipLevels = 0;
    texDesc.ArraySize = 6;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

    ID3D11Texture2D* cubeTex = 0;
    mDev->CreateTexture2D(&texDesc, 0, &cubeTex);

    // Create a render target view to each cube map face (i.e., each element in the texture array).
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = texDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    rtvDesc.Texture2DArray.ArraySize = 1;
    rtvDesc.Texture2DArray.MipSlice = 0;

    for(int i = 0; i < 6; ++i)
    {
        rtvDesc.Texture2DArray.FirstArraySlice = i;
        mDev->CreateRenderTargetView(cubeTex, &rtvDesc, &mDynamicCubeMapRTVSphere[i]);
    }

    // Create a shader resource view to the cube map.
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = -1;

   HRESULT hrte = mDev->CreateShaderResourceView(cubeTex, &srvDesc, &mDynamicCubeMapSRVSphere);

    //ReleaseCOM(cubeTex);

    // We need a depth texture for rendering the scene into the cubemap that has the same resolution as the cubemap faces.  
    D3D11_TEXTURE2D_DESC depthTexDesc;
    depthTexDesc.Width = CubeMapSizeSphere;
    depthTexDesc.Height = CubeMapSizeSphere;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTexDesc.CPUAccessFlags = 0;
    depthTexDesc.MiscFlags = 0;

    ID3D11Texture2D* depthTex = 0;
    mDev->CreateTexture2D(&depthTexDesc, 0, &depthTex);

    // Create the depth stencil view for the entire cube
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format = depthTexDesc.Format;
    dsvDesc.Flags  = 0;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    mDev->CreateDepthStencilView(depthTex, &dsvDesc, &mDynamicCubeMapDSVSphere);

    //ReleaseCOM(depthTex);

    // Viewport for drawing into cubemap.
    mCubeMapViewport.TopLeftX = 0.0f;
    mCubeMapViewport.TopLeftY = 0.0f;
    mCubeMapViewport.Width    = (float)CubeMapSizeSphere;
    mCubeMapViewport.Height   = (float)CubeMapSizeSphere;
    mCubeMapViewport.MinDepth = 0.0f;
    mCubeMapViewport.MaxDepth = 1.0f;
}