#include "RenderManager.h"
#include "GeometryGenerator.h"


RenderManager::RenderManager(ID3D11DeviceContext *devcon, 
	ID3D11Device *dev, 
	IDXGISwapChain *swapchain,
	Apex *apex,
	Camera *cam,
	D3D11_VIEWPORT *viewport) :
mDevcon(devcon), mDev(dev), mSwapchain(swapchain), mCam(cam), mApex(apex), mViewport(viewport)
{
	sceneBuff.normalMap		= 1;
	sceneBuff.phong			= 1;
	sceneBuff.textures		= 1;
	sceneBuff.ambientOn		= 1;
	sceneBuff.diffuseOn		= 1;
	sceneBuff.specularOn	= 1;
	sceneBuff.dirLightOn	= 1;
	sceneBuff.pLightOn		= 1;

	emitterOn = true;

	fps = 0;
	SCREEN_WIDTH = 1280;
	SCREEN_HEIGHT = 720;

	ID3D11Texture2D *pBackBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	dev->CreateRenderTargetView(pBackBuffer, NULL, &mBackbuffer);
	pBackBuffer->Release();

	gRenderer = new ZeusRenderer();

	GeometryGenerator *geoGen = new GeometryGenerator();

	mSkyBox = new SkyBox(mDevcon, mDev, geoGen);

	mCloth = apex->CreateCloth(gRenderer, "curtainew");//"ctdm_Cape_400");//"bannercloth");//
	renderables.push_back(mCloth);

	emitter = apex->CreateEmitter(gRenderer, "SmokeEmitter");
	emitter->SetPosition(-18.0f, -65.0f, -243.0f);
	//emitter->SetEmit(true);

    sphere2 = new Sphere(mDevcon, mDev, geoGen, apex, 2, 30, 30);
	renderables.push_back(sphere2);
	sphere2->MoveTo(0.0f,5.0f,-10.0f);//-18.0f, -65.0f, -243.0f);//-3.0f, 309.5f, -957.3f);
    
	torch1 = apex->CreateEmitter(gRenderer, "TorchEmitter");
	torch1->SetPosition(-13.5f, -2.0f, -42.0f);
    torch2 = apex->CreateEmitter(gRenderer, "TorchEmitter");
    torch2->SetPosition(-67.6f, -2.0f, -42.0f);

	particles = apex->CreateEmitter(gRenderer, "testSpriteEmitter4ParticleFluidIos");
    particles->SetPosition(-19.0f, 45.0f, 206.0f);

    mSphereMove = new Sphere(mDevcon, mDev, geoGen, apex, 2, 30, 30);
	renderables.push_back(mSphereMove);
	mSphereMove->MoveTo(-19.0f, 45.0f, 206.0f);
	
	//Special "renderable" case, do not add to the vector
	mScreen = new ScreenQuad(mDevcon, mDev, geoGen);
	//Special camera, doesn't move

	Scene* scene = new Scene(&renderables, dev, devcon, apex);

	projectile = new Projectile(dev, devcon, apex);
	renderables.push_back(projectile);

	mGrid = new GroundPlane(mDevcon, mDev, geoGen, 400, 10);
	//renderables.push_back(mGrid);

	HRESULT hr;

	hr = mFont.Initialize(mDev, L"Times New Roman", 30.0f, FontSheet::FontStyleRegular, true);
	hr = mText.Initialize(mDev);


	mScreenCam = new Camera();
	mScreenCam->SetLensOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1000.0f);
	mScreenCam->UpdateViewMatrix();

	free(geoGen);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SceneBuff);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	mDev->CreateBuffer(&bd, NULL, &sceneCBuffer);


	// create the depth buffer texture
	ID3D11RasterizerState*		pState;
	D3D11_RASTERIZER_DESC		raster;
	ZeroMemory( &raster, sizeof(D3D11_RASTERIZER_DESC));

	raster.FillMode = D3D11_FILL_SOLID;
	raster.CullMode = D3D11_CULL_NONE;
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

	D3D11_SAMPLER_DESC sd;
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.MaxAnisotropy = 16;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.BorderColor[0] = 0.0f;
	sd.BorderColor[1] = 0.0f;
	sd.BorderColor[2] = 0.0f;
	sd.BorderColor[3] = 0.0f;
	sd.MinLOD = 0.0f;
	sd.MaxLOD = FLT_MAX;
	sd.MipLODBias = 0.0f;

	dev->CreateSamplerState(&sd, &mSampState);
	devcon->PSSetSamplers(0, 1, &mSampState);

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

	mDirLight[0].Ambient =		XMFLOAT4(.3f, .3f, .3f, 1);
	mDirLight[0].Diffuse =		XMFLOAT4(.5f, .5f, .5f, 1);
	mDirLight[0].Direction =	XMFLOAT3(0.57735f, -0.68f, -0.866f);
	mDirLight[0].Specular =		XMFLOAT4(0.9f, 0.9f, 0.9f, 1);

	mDirLight[1].Ambient =		XMFLOAT4(.3f, .3f, .3f, 1);
	mDirLight[1].Diffuse =		XMFLOAT4(.6f, .6f, .6f, 1);
	mDirLight[1].Direction =	XMFLOAT3(10, 0, 0);
	mDirLight[1].Specular =		XMFLOAT4(1, 1, 1, 1);


	//Set the point light
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(PointLight) * 2;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &pointLightCBuffer);

	mPointLight[0].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
	mPointLight[0].Att      = XMFLOAT3(1.0f, 0.05f, .0075f);
	mPointLight[0].Diffuse  = XMFLOAT4(1.7f, 0.6f, 0.0f, 1);
	mPointLight[0].Specular = XMFLOAT4(1, 1, 1, 1);
	mPointLight[0].Range    = 15.0f;
	mPointLight[0].Position = XMFLOAT3(-13.5f, -3.0f, -42.0f);

	mPointLight[1].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
	mPointLight[1].Att      = XMFLOAT3(1.0f, 0.05f, .0075f);
	mPointLight[1].Diffuse  = XMFLOAT4(1.7f, 0.6f, 0.0f, 1);
	mPointLight[1].Specular = XMFLOAT4(1, 1, 1, 1);
	mPointLight[1].Range    = 15.0f;
	mPointLight[1].Position = XMFLOAT3(-67.7f, -3.0f, -42.0f);

	mSphere = new Sphere(mDevcon, mDev, geoGen, apex, 4, 60, 60);
	renderables.push_back(mSphere);
	mSphere->SetupReflective(&renderables, mSkyBox, mScreen, mZbuffer, mViewport);
	mSphere->MoveTo(-37.0f, -12.0f, -85.0f);

	renderables.push_back(emitter);
	renderables.push_back(particles);
	renderables.push_back(torch1);
	renderables.push_back(torch2);

	// Shadow Initialization

	// Set the viewport
	ZeroMemory(&mShadowPort, sizeof(D3D11_VIEWPORT));

	mShadowPort.TopLeftX = 0;
	mShadowPort.TopLeftY = 0;
	mShadowPort.Width = 2048;
	mShadowPort.Height = 2048;
	mShadowPort.MaxDepth = 1;
	mShadowPort.MinDepth = 0;

	mShadowCam = new Camera();
	//Set the shadow positions
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ShadowBuff);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	mDev->CreateBuffer(&bd, NULL, &shadowCBuffer);

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

	hr = mDev->CreateTexture2D( &descDepth, NULL, &pShadowMap );
	if( FAILED(hr) )  
		return ;
	hr = mDev->CreateDepthStencilView( pShadowMap, &descDSV2, &pShadowMapDepthView );
	if( FAILED(hr) )  
		return;
	hr = mDev->CreateShaderResourceView( pShadowMap, &srvDesc, &pShadowMapSRView);
	if( FAILED(hr) )  
		return;

	ID3D11SamplerState* pSS;
	//	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	//sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sd.MaxAnisotropy = 1;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR       ;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR       ;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR       ;

	sd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	mDev->CreateSamplerState(&sd, &pSS);
	mDevcon->PSSetSamplers(5, 1, &pSS);
}


void RenderManager::fpsCalc(GameTimer mTimer)
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if( (mTimer.TotalTime() - timeElapsed) >= 1.0f )
	{
		fps = (int)frameCnt; // fps = frameCnt / 1
		mspf = 1000.0f / fps;

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}


float timePassed = 0.0f;
void RenderManager::Update(float dt)
{
	timePassed += dt;
	// Other animation?
	float x,y,z;
	x = 200.0f * (float)sin((float)timePassed);
	y = abs(50.f * (float)sin((float)timePassed/0.3f))-10.0f;
	z = 200.0f * (float)cos((float)timePassed);

	//SetPosition(x,y,z);
	//mSphereMove->MoveTo(x,y,z);
	projectile->Update();
	particles->Update();
	emitter->Update();
	torch1->Update();
	torch2->Update();
	mCloth->Update();
}

void RenderManager::GetScreenParams(int mClientWidth, int mClientHeight)
{
	SCREEN_WIDTH = mClientWidth;
	SCREEN_HEIGHT = mClientHeight;
}

void RenderManager::Render()
{
	char buf[50];
	itoa(fps, buf, 10);
	sText = std::string("FPS: ") + buf;
	std::string hair = "+";

	// Convert the pos to a w string
	std::string xs,ys,zs;
	std::stringstream posx,posy,posz;
	posx << mCam->GetPosition().x;
	xs = posx.str();
	posy << mCam->GetPosition().y;
	ys = posy.str();
	posz << mCam->GetPosition().z;
	zs = posz.str();
	std::string pos = xs + ", " + ys + ", " + zs;

	// Calculate the text width.
	int textWidth = 0;
	for(UINT i = 0; i < sText.length(); ++i)
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

	// Calculate the hair width.
	int hairWidth = 0;
	for(UINT i = 0; i < hair.size(); ++i)
	{
		WCHAR character = hair[i];
		if(character == ' ') 
		{
			hairWidth += mFont.GetSpaceWidth();
		}
		else{
			const CD3D11_RECT& r = mFont.GetCharRect(hair[i]);
			hairWidth += (r.right - r.left);
		}
	}

	// Calculate the pos width
	int posWidth = 0;
	for(UINT i = 0; i < pos.size(); ++i)
	{
		WCHAR character = pos[i];
		if(character == ' ') 
		{
			hairWidth += mFont.GetSpaceWidth();
		}
		else{
			const CD3D11_RECT& r = mFont.GetCharRect(pos[i]);
			posWidth += (r.right - r.left + 1);
		}
	}


	textPos.x = (SCREEN_WIDTH - textWidth) - 2.0;
	textPos.y = 0;//SCREEN_HEIGHT;

	hairPos.x = (SCREEN_WIDTH - hairWidth) / 2;
	hairPos.y = (SCREEN_HEIGHT - mFont.GetCharHeight()) / 2 ;

	posPos.x = 2.0;
	posPos.y = 1.0;


	// clear the back buffer to a deep blue
	mDevcon->ClearDepthStencilView(mZbuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	mDevcon->ClearRenderTargetView(mBackbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));
	mDevcon->ClearRenderTargetView(mScreen->mTargetView, D3DXCOLOR(0.0f, 1.0f, 0.4f, 1.0f));

	mDevcon->ClearDepthStencilView(pShadowMapDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//mDevcon->RSSetState(0);

	XMStoreFloat4x4(&sceneBuff.viewProj, mCam->ViewProj());
	sceneBuff.camPos = mCam->GetPosition();
	//sceneBuff.pad = 1.0f;
	mDevcon->VSSetConstantBuffers(0, 1, &sceneCBuffer);
	mDevcon->PSSetConstantBuffers(0, 1, &sceneCBuffer);

	mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, &sceneBuff , 0, 0);

	//Skybox right now doesn't like zbuffers, so dont' set one for it
	mDevcon->OMSetRenderTargets(1, &mScreen->mTargetView, 0);
	mSkyBox->Render(sceneCBuffer, mCam, 0);


	mDevcon->OMSetRenderTargets(1, &mScreen->mTargetView/*mBackbuffer*/, mZbuffer);

	mDevcon->PSSetConstantBuffers(2, 1, &dirLightCBuffer);
	mDevcon->UpdateSubresource(dirLightCBuffer, 0, 0, &mDirLight, 0, 0);

	mDevcon->PSSetConstantBuffers(3, 1, &pointLightCBuffer);
	mDevcon->UpdateSubresource(pointLightCBuffer, 0, 0, &mPointLight, 0, 0);

	// START SHADOW MAP
	mDevcon->RSSetViewports(1, &mShadowPort);
	mShadowCam->SetLensOrtho( -350.0, 350.0, -350.0, 350.0, 1.0, 1000);
	float distanceMult = 550.0f;
	XMFLOAT3 ShadPos = XMFLOAT3( (-mDirLight[0].Direction.x * distanceMult) + mCam->GetPosition().x, 
		(-mDirLight[0].Direction.y * distanceMult) + mCam->GetPosition().y,
		(-mDirLight[0].Direction.z * distanceMult) + mCam->GetPosition().z ); 

	mShadowCam->SetPosition(ShadPos);
	mShadowCam->LookAt( ShadPos, mCam->GetPosition(), XMFLOAT3(0,1,0));
	mShadowCam->UpdateViewMatrix();


	sceneBuff.camPos = mShadowCam->GetPosition();

	XMStoreFloat4x4(&sceneBuff.viewProj, mShadowCam->ViewProj()); 

	mDevcon->VSSetConstantBuffers(0, 1, &sceneCBuffer);
	mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, &sceneBuff , 0, 0);
	shadowBuff.lightPos = mShadowCam->GetPosition();
	XMStoreFloat4x4( &shadowBuff.lightViewProj,  XMMatrixMultiply(mShadowCam->View(), mShadowCam->Proj()) );
	//XMStoreFloat4x4( &shadowBuff.lightViewProj,  XMMatrixMultiply(mShadowCam->View(), mCam->Proj()) );

	shadowBuff.PADdyCake = .5;

	mDevcon->PSSetConstantBuffers(5, 1, &shadowCBuffer);
	mDevcon->VSSetConstantBuffers(5, 1, &shadowCBuffer);
	mDevcon->UpdateSubresource(shadowCBuffer, 0, 0, &shadowBuff , 0, 0);
	//*********************


	mDevcon->OMSetRenderTargets(0, 0, pShadowMapDepthView);
	//obj->Depth(sceneCBuffer, mCam, depth);
	for(int i = 0; i < renderables.size() ; i++)
	{
		renderables[i]->Depth();
	}

	mDevcon->OMSetRenderTargets(1, &mScreen->mTargetView/*mBackbuffer*/, mZbuffer);

	// MUST PASS IN THE SHADER RESOURCE VIEW AFTER YOU SWITCH TEH RENDER TARGET OTHERWISE ITS NULL
	mDevcon->PSSetShaderResources(6, 1, &pShadowMapSRView);

	//***********************
	XMStoreFloat4x4(&sceneBuff.viewProj, mCam->ViewProj());

	sceneBuff.camPos = mCam->GetPosition();

	mDevcon->VSSetConstantBuffers(0, 1, &sceneCBuffer);
	mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, &sceneBuff , 0, 0);
	//***********************
	// END SHADOW MAP
	mDevcon->RSSetViewports(1, mViewport);
	Render(0);


	//Render the screen quad last
	mDevcon->OMSetRenderTargets(1, &mBackbuffer, 0);

	mDevcon->PSSetShaderResources(1, 1, &mDepthShaderResourceView);
	mDevcon->PSSetShaderResources(6, 1, &pShadowMapSRView);

	mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, mScreenCam->ViewProj().m , 0, 0);
	mScreen->Render(sceneCBuffer, mScreenCam, 0);

	mText.DrawString(mDevcon, mFont, sText, textPos, XMCOLOR(0xffffffff));
	mText.DrawString(mDevcon, mFont, hair, hairPos, XMCOLOR(0xffffffff));
	mText.DrawString(mDevcon, mFont, pos, posPos, XMCOLOR(0xffffffff));

	//if(!((mCam->GetPosition().x > 6.0f || mCam->GetPosition().x < -44.0) ||
	//	(mCam->GetPosition().y > 70.0f || mCam->GetPosition().y < 20.0) ||
	//	(mCam->GetPosition().z > 231.0f || mCam->GetPosition().z < 181.0)) ){
			mSphere->IsItReflective(true);
	/*}
	else{
		mSphere->IsItReflective(false);
	}*/
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


//DEBUG
void RenderManager::SetPosition(float x, float y, float z)
{
	//particles->SetPosition(x,y,z);
	//emitter->SetPosition(-3.0f, 309.5f, -957.3);
}

void RenderManager::SetEmit(bool on)
{
	particles->SetEmit(on);
	emitter->SetEmit(on);
}

void RenderManager::RecompShaders()
{
	mScreen->RecompileShader();

	for(int i = 0; i < renderables.size() ; i++)
	{
		renderables[i]->RecompileShader();
	}
}
