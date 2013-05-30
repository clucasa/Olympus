#include "RenderManager.h"
#include "GeometryGenerator.h"
#include <stdlib.h>     /* srand, rand */

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
    sceneBuff.shadowsOn		= 1;
    sceneBuff.cascadeOn		= 0;


    PartyMode = 0;

    emitterOn = true;

    fps = 0;
    SCREEN_WIDTH = 1280;
    SCREEN_HEIGHT = 720;

    ID3D11Texture2D *pBackBuffer;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // use the back buffer address to create the render target
    dev->CreateRenderTargetView(pBackBuffer, NULL, &mBackbuffer);
    pBackBuffer->Release();

    mAssetManager = new AssetManager(mDev, mDevcon);

    GeometryGenerator *geoGen = new GeometryGenerator();

    //Special "renderable" case, do not add to the vector
    mScreen = new ScreenQuad(mDevcon, mDev, geoGen);
    //Special camera, doesn't move

    mSkyBox = new SkyBox(mDevcon, mDev, geoGen);



    //mCloth = apex->CreateCloth(gRenderer, "curtainew");//"ctdm_Cape_400");//"bannercloth");//
    //renderables.push_back(mCloth);

    //emitter = apex->CreateEmitter(gRenderer, "SmokeEmitter");
    //emitter->SetPosition(-18.0f, -65.0f, -243.0f);
    ////emitter->SetEmit(true);

    //   
    //torch1 = apex->CreateEmitter(gRenderer, "TorchEmitter");
    //torch1->SetPosition(-13.5f, -2.0f, -42.0f);
    //   torch2 = apex->CreateEmitter(gRenderer, "TorchEmitter");
    //   torch2->SetPosition(-67.6f, -2.0f, -42.0f);

    //particles = apex->CreateEmitter(gRenderer, "testSpriteEmitter4ParticleFluidIos");
    //   particles->SetPosition(-19.0f, 45.0f, 206.0f);

    /*projectile = new Projectile(dev, devcon, apex);
    renderables.push_back(projectile);*/

    //mGrid = new GroundPlane(mDevcon, mDev, geoGen, 4000, 10);
    //renderables.push_back(mGrid);


    //renderables.push_back(emitter);
    //renderables.push_back(particles);
    //renderables.push_back(torch1);
    //renderables.push_back(torch2);



    HRESULT hr;

    hr = mFont.Initialize(mDev, L"Times New Roman", 30.0f, FontSheet::FontStyleRegular, true);
    hr = mText.Initialize(mDev);


    mScreenCam = new Camera();
    mScreenCam->SetLensOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1000.0f);
    mScreenCam->UpdateViewMatrix();

    
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SceneBuff);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &sceneCBuffer);


    // create the depth buffer texture
    //ID3D11RasterizerState*	pState;
    //D3D11_RASTERIZER_DESC		raster;

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
    bd.ByteWidth = sizeof(DirectionalLight)*10;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &dirLightCBuffer);

    mDirLight[0].Ambient =		XMFLOAT4(.4f, .4f, .4f, 1);
    mDirLight[0].Diffuse =		XMFLOAT4(.6f, .6f, .6f, 1);
    mDirLight[0].Direction =	XMFLOAT3(0.57735f, -0.68f, -0.866f);
    mDirLight[0].Specular =		XMFLOAT4(0.7f, 0.67f, 0.62f, 1);

    mDirLight[1].Ambient =		XMFLOAT4(.3f, .3f, .3f, 1);
    mDirLight[1].Diffuse =		XMFLOAT4(.6f, .6f, .6f, 1);
    mDirLight[1].Direction =	XMFLOAT3(10, 0, 0);
    mDirLight[1].Specular =		XMFLOAT4(1, 1, 1, 1);


    //Set the point light
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(PointLight) * MAX_NUM_POINT_LIGHTS;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &pointLightCBuffer);



    ID3D11SamplerState* pSS;
    //	D3D11_SAMPLER_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    //sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    sd.MaxAnisotropy = 1;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;

    sd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    mDev->CreateSamplerState(&sd, &pSS);
    mDevcon->PSSetSamplers(5, 1, &pSS);

    vector<String> sceneNames;
    sceneNames.push_back("scene/hub_scene.txt");
    sceneNames.push_back("scene/bowling_scene.txt");
    sceneNames.push_back("scene/dark_scene.txt");
    sceneNames.push_back("scene/jenga_scene.txt");
    sceneNames.push_back("scene/openworld_scene.txt");



    Scene *tempScene;
    newTimer.Reset();
    for(int i = 0; i < (int)sceneNames.size(); i++)
    {
        apex->setScene(i);
        tempScene = new Scene( dev, devcon, apex, geoGen, mSkyBox, mScreen, mZbuffer, mViewport, sceneNames[i], mAssetManager);
        scene.push_back(tempScene);
        newTimer.Tick();
        sceneLoadTimes.push_back(newTimer.DeltaTime());
    }


    // Setting up Cascade Light Shadow Maps
    for( int i = 0; i < scene.size(); i++ )
    {
        if( i == 0)
            shad = new ShadowManager(mDev, mDevcon, 2048, 2048, 0, 25.0, 25.0);
        else if( i == 1)
            shad = new ShadowManager(mDev, mDevcon, 2048, 2048, 0, 50.0, 50.0);
        else if( i == 2)
            shad = new ShadowManager(mDev, mDevcon, 2048, 2048, 0, 60.0, 60.0);
        else if( i == 3)
            shad = new ShadowManager(mDev, mDevcon, 2048, 2048, 0, 120.0, 120.0);
        else
            shad = new ShadowManager(mDev, mDevcon, 2048, 2048, 0, 60.0, 60.0);

        shad->lightPos = mDirLight[0].Direction;
        shadows[i].push_back( shad );
    }

    for( int i = 0; i < scene.size(); i++ )
    {
        shad = new ShadowManager(mDev, mDevcon, 1024, 1024, 0, 120.0, 120.0);

        if( i == 0)
            shad = new ShadowManager(mDev, mDevcon, 1024, 1024, 0, 50.0, 50.0);
        else if( i == 1)
            shad = new ShadowManager(mDev, mDevcon, 1024, 1024, 0, 100.0, 100.0);
        else if( i == 2)
            shad = new ShadowManager(mDev, mDevcon, 1024, 1024, 0, 120.0, 120.0);
        else if( i == 3)
            shad = new ShadowManager(mDev, mDevcon, 1024, 1024, 0, 250.0, 250.0);
        else
            shad = new ShadowManager(mDev, mDevcon, 1024, 1024, 0, 120.0, 120.0);

        shad->lightPos = mDirLight[0].Direction;
        shadows[i].push_back( shad );
    }
    
    for( int i = 0; i < scene.size(); i++ )
    {
            shad = new ShadowManager(mDev, mDevcon, 1024, 1024, 0, 120.0, 120.0);

        if( i == 0)
            shad = new ShadowManager(mDev, mDevcon, 512, 512, 0, 300.0, 300.0);
        else if( i == 1)
            shad = new ShadowManager(mDev, mDevcon, 512, 512, 0, 300.0, 300.0);
        else if( i == 2)
            shad = new ShadowManager(mDev, mDevcon, 512, 512, 0, 300.0, 300.0);
        else if( i == 3)
            shad = new ShadowManager(mDev, mDevcon, 512, 512, 0, 500.0, 500.0);
        else
            shad = new ShadowManager(mDev, mDevcon, 512, 512, 0, 300.0, 300.0);
        shad->lightPos = mDirLight[0].Direction;
        shadows[i].push_back( shad );
    }

    //// Setting up Point Light Shadow Maps
    //for( int i = 1; i <= 6; i++ )
    //{
    //	shad = new ShadowManager(mDev, mDevcon, 512, 512, i);
    //	shad->lightPos = mPointLight[0].Position;
    //	shadows.push_back( shad );
    //}

    //for( int i = 1; i <= 6; i++ )
    //{
    //	shad = new ShadowManager(mDev, mDevcon, 512, 512, i);
    //	shad->lightPos = mPointLight[1].Position;
    //	shadows.push_back( shad );
    //}

    


    PointLight tempPointLight;

    //SCENE0 - HUB
    tempPointLight.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
    tempPointLight.Att      = XMFLOAT3(1.0f, 0.05f, .0075f);
    tempPointLight.Diffuse  = XMFLOAT4(1.7f, 0.6f, 0.0f, 1);
    tempPointLight.Specular = XMFLOAT4(1, 1, 1, 1);
    tempPointLight.Range    = 15.0f;
    tempPointLight.Position = XMFLOAT3(-22.7255f, -11.34f, -15.337f);


    scene[0]->mPointLights[0] =  tempPointLight;
    scene[0]->mPointLights[0].Pad =  2;

    tempPointLight.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
    tempPointLight.Att      = XMFLOAT3(1.0f, 0.05f, .0075f);
    tempPointLight.Diffuse  = XMFLOAT4(1.7f, 0.6f, 0.0f, 1);
    tempPointLight.Specular = XMFLOAT4(1, 1, 1, 1);
    tempPointLight.Range    = 15.0f;
    tempPointLight.Position = XMFLOAT3(-11.8275f, -11.34f, -26.35f);

    scene[0]->mPointLights[1] = tempPointLight;


    //SCENE1 - BOWLING
    XMFLOAT3 startPos1(-25.0f, 10.0f, -160);
    XMFLOAT3 startPos2( 25.0f, 10.0f, -160);
    
    

    for(int i = 0; i < 10; i++)
    {
        tempPointLight.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
        tempPointLight.Att      = XMFLOAT3(1.0f, 0.05f, .0075f);
        tempPointLight.Diffuse  = Colors::Green;
        tempPointLight.Specular = XMFLOAT4(1, 1, 1, 1);
        tempPointLight.Range    = 30.0f;
        tempPointLight.Position = startPos1;

        startPos1.z += 20.0f;

        scene[1]->mPointLights[i] = tempPointLight;
    }

    for(int i = 10; i < 20; i++)
    {
        tempPointLight.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
        tempPointLight.Att      = XMFLOAT3(1.0f, 0.05f, .0075f);
        tempPointLight.Diffuse  = Colors::Green;
        tempPointLight.Specular = XMFLOAT4(1, 1, 1, 1);
        tempPointLight.Range    = 30.0f;
        tempPointLight.Position = startPos2;

        startPos2.z += 20.0f;

        scene[1]->mPointLights[i] = tempPointLight;
    }

        tempPointLight.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
        tempPointLight.Att      = XMFLOAT3(1.0f, 0.05f, .0075f);
        tempPointLight.Diffuse  = Colors::Green;
        tempPointLight.Specular = XMFLOAT4(1, 1, 1, 1);
        tempPointLight.Range    = 25.0f;
        tempPointLight.Position = XMFLOAT3(0.0f, 16.0f, 27.5f);

        scene[1]->mPointLights[20] = tempPointLight;

        scene[1]->mPointLights[0].Pad = 21;

    for( int i = 0; i < 1; i++ )
    {
        for( int j = 1; j <= 6; j++ )
        {
            shad = new ShadowManager(mDev, mDevcon, 512, 512, j);
            shad->lightPos = scene[1]->mPointLights[i].Position;
            shadows[1].push_back( shad );
        }
    }


    ////SCENE2 - DARKNESS
    //tempPointLight.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
    //tempPointLight.Att      = XMFLOAT3(1.0f, 0.05f, .0075f);
    //tempPointLight.Diffuse  = Colors::Magenta;
    //tempPointLight.Specular = XMFLOAT4(1, 1, 1, 1);
    //tempPointLight.Range    = 15.0f;
    //tempPointLight.Position = XMFLOAT3(-0.5f, 2.5f, 73.0f);

    //scene[2]->mPointLights[0] = tempPointLight;
    //scene[2]->mPointLights[0].Pad = 1;

    //SCENE2 - DARKNESS
    tempPointLight.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
    tempPointLight.Att      = XMFLOAT3(1.0f, 0.05f, .0075f);
    tempPointLight.Diffuse  = Colors::TorchOrange;
    tempPointLight.Specular = XMFLOAT4(1, 1, 1, 1);
    tempPointLight.Range    = 15.0f;
    tempPointLight.Position = XMFLOAT3(-0.5f, 2.5f, 73.0f);

    scene[2]->mPointLights[0] = tempPointLight;
    scene[2]->mPointLights[0].Pad = 1;

    /*tempPointLight.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
    tempPointLight.Att      = XMFLOAT3(1.0f, 0.05f, .0075f);
    tempPointLight.Diffuse  = Colors::Magenta;
    tempPointLight.Specular = XMFLOAT4(1, 1, 1, 1);
    tempPointLight.Range    = 15.0f;
    tempPointLight.Position = XMFLOAT3(0.0f, 14.0f, 180.0f);

    scene[2]->mPointLights[1] = tempPointLight;*/

    for( int i = 0; i < 2; i++ )
    {
        for( int j = 1; j <= 6; j++ )
        {
            shad = new ShadowManager(mDev, mDevcon, 1024, 1024, j);
            shad->lightPos = scene[2]->mPointLights[i].Position;
            shadows[2].push_back( shad );
        }
    }

    //SCENE3 - 

    tempPointLight.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
    tempPointLight.Att      = XMFLOAT3(1.0f, 0.05f, .0075f);
    tempPointLight.Diffuse  = Colors::Blue;
    tempPointLight.Specular = XMFLOAT4(1, 1, 1, 1);
    tempPointLight.Range    = 15.0f;
    tempPointLight.Position = XMFLOAT3(0.0f, 4.5f, 17.0f);

    scene[3]->mPointLights[0] = tempPointLight;
    scene[3]->mPointLights[0].Pad = 1;


    //SCENE4 - OPEN WORLD
    scene[4]->mPointLights[0].Pad = 0;


    apex->setScene(0);
    

    // Shadow Constant Buffer
    for( int j = 0; j < scene.size(); j++ )
    {
        ZeroMemory(&bd, sizeof(bd));

        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(ShadowBuff) * shadows[j].size();
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        mDev->CreateBuffer(&bd, NULL, &shadowCBuffer[j]);

        for( int i = 0; i < shadows[i].size(); i++ )
        {
            ShadowBuff temp;
            temp.lightPos = shadows[j][i]->lightPos;
            XMStoreFloat4x4( &temp.lightViewProj, shadows[j][i]->mShadowCam->ViewProj() );
            shadowBuff[j].push_back( temp );
        }
    }
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

    scene[mCurrentScene]->Update();

}

void RenderManager::GetScreenParams(int mClientWidth, int mClientHeight)
{
    SCREEN_WIDTH = mClientWidth;
    SCREEN_HEIGHT = mClientHeight;
}


int cosmicFactor = 0;

void RenderManager::Render()
{
    char buf[50];
    _itoa_s(fps, buf, 10);
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

    //COSMIC BOOOWLAN
    if((mCurrentScene == CurrentScene::BOWLING) && (cosmicFactor % 41 == 0) && PartyMode == 1)
    {
        int colorNum;

        for(int i = 0; i < 21; i++)
        {
            srand(cosmicFactor++);
            colorNum = rand() % 5;
            
            if(colorNum == 0)
                scene[mCurrentScene]->mPointLights[i].Diffuse = Colors::Red; 
            if(colorNum == 1)
                scene[mCurrentScene]->mPointLights[i].Diffuse = Colors::Blue;  
            if(colorNum == 2)
                scene[mCurrentScene]->mPointLights[i].Diffuse = Colors::Green;  
            if(colorNum == 3)
                scene[mCurrentScene]->mPointLights[i].Diffuse = Colors::Cyan;  
            if(colorNum == 4)
                scene[mCurrentScene]->mPointLights[i].Diffuse = Colors::Magenta; 

            scene[mCurrentScene]->mPointLights[i].Ambient = Colors::Black; 
            scene[mCurrentScene]->mPointLights[i].Range   = 30.0f;
        }
    }
    else if((mCurrentScene == CurrentScene::BOWLING) && (PartyMode == 0))
    {
        for(int i = 0; i < 21; i++)
        {
            scene[mCurrentScene]->mPointLights[i].Diffuse = Colors::NormalDiffuse; 
            scene[mCurrentScene]->mPointLights[i].Ambient = Colors::NormalAmbient; 	
            scene[mCurrentScene]->mPointLights[i].Range   = 40.0f; 	
        }

        PartyMode = -1;	
    }


    cosmicFactor++;


    textPos.x = (LONG)((SCREEN_WIDTH - textWidth) - 2.0f);
    textPos.y = 0;//SCREEN_HEIGHT;

    hairPos.x = (LONG)((SCREEN_WIDTH - hairWidth) / 2.0f);
    hairPos.y = (LONG)((SCREEN_HEIGHT - mFont.GetCharHeight()) / 2.0f) ;

    posPos.x = (LONG)2.0f;
    posPos.y = (LONG)1.0f;


    // clear the back buffer to a deep blue
    mDevcon->ClearDepthStencilView(mZbuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    mDevcon->ClearRenderTargetView(mBackbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));
    mDevcon->ClearRenderTargetView(mScreen->mTargetView, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

    for( int j = 0; j < scene.size(); j++ )
        for( int i = 0; i < shadows[j].size(); i++ )
            mDevcon->ClearDepthStencilView(shadows[j][i]->pShadowMapDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    //mDevcon->RSSetState(0);

    XMStoreFloat4x4(&sceneBuff.viewProj, mCam->ViewProj());
    sceneBuff.camPos = mCam->GetPosition();
    //sceneBuff.pad = 1.0f;
    mDevcon->VSSetConstantBuffers(0, 1, &sceneCBuffer);
    mDevcon->PSSetConstantBuffers(0, 1, &sceneCBuffer);

    mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, &sceneBuff , 0, 0);

    if(mCurrentScene != CurrentScene::DARKNESS && mCurrentScene != CurrentScene::BOWLING)
    {
        //Skybox right now doesn't like zbuffers, so dont' set one for it
        mDevcon->OMSetRenderTargets(1, &mScreen->mTargetView, 0);
        mSkyBox->Render(sceneCBuffer, mCam, 0);
    }

    if(mCurrentScene == CurrentScene::DARKNESS)
    {
        scene[mCurrentScene]->mPointLights[0].Position = mCam->GetPosition();
        
        for(int i = 0; i < 2; i++)
        {
            scene[mCurrentScene]->mPointLights[i].Range = Randomf(20.0f, 21.0f);
        }
    }

    mDevcon->OMSetRenderTargets(1, &mScreen->mTargetView/*mBackbuffer*/, mZbuffer);

    mDevcon->PSSetConstantBuffers(2, 1, &dirLightCBuffer);
    mDevcon->UpdateSubresource(dirLightCBuffer, 0, 0, &mDirLight, 0, 0);

    mDevcon->PSSetConstantBuffers(3, 1, &pointLightCBuffer);
    mDevcon->UpdateSubresource(pointLightCBuffer, 0, 0, &scene[mCurrentScene]->mPointLights, 0, 0);
    
    //raster.CullMode = D3D11_CULL_FRONT;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //mDevcon->RSSetState( pState );
    // START SHADOW MAP
    RenderToShadow();

    mDevcon->OMSetRenderTargets(1, &mScreen->mTargetView/*mBackbuffer*/, mZbuffer);

    // MUST PASS IN THE SHADER RESOURCE VIEW AFTER YOU SWITCH TEH RENDER TARGET OTHERWISE ITS NULL
    SetShadowSRV( 6 );

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
    SetShadowSRV( 6 );
    
    //mDevcon->PSSetShaderResources(6, 1, &pShadowMapSRView);

    mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, mScreenCam->ViewProj().m , 0, 0);
    mScreen->Render(sceneCBuffer, mScreenCam, 0);

    //mText.DrawString(mDevcon, mFont, sText, textPos, XMCOLOR(0xffffffff));
    mText.DrawString(mDevcon, mFont, hair, hairPos, XMCOLOR(0xffffffff));
    mText.DrawString(mDevcon, mFont, pos, posPos, XMCOLOR(0xffffffff));

    //if(!((mCam->GetPosition().x > 6.0f || mCam->GetPosition().x < -44.0) ||
    //	(mCam->GetPosition().y > 70.0f || mCam->GetPosition().y < 20.0) ||
    //	(mCam->GetPosition().z > 231.0f || mCam->GetPosition().z < 181.0)) ){
    //mSphere->IsItReflective(true);
    /*}
    else{
    mSphere->IsItReflective(false);
    }*/
}

void RenderManager::Render(int renderType)
{
    for(int i = 0; i < (int)scene[mCurrentScene]->mRenderables.size() ; i++)
    {
        scene[mCurrentScene]->mRenderables[i]->Render(sceneCBuffer, mCam, renderType);
    }
    for(int i = 0; i < (int)scene[mCurrentScene]->mBlendRenderables.size() ; i++)
    {
        scene[mCurrentScene]->mBlendRenderables[i]->Render(sceneCBuffer, mCam, renderType);
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


void RenderManager::RenderToShadow()
{
    for( int i = 0; i < shadows[mCurrentScene].size(); i++ )
    {
        if( shadows[mCurrentScene][i]->viewType == 0 || shadows[mCurrentScene][i]->viewType == 8)
            shadows[mCurrentScene][i]->SetConstantBuffer(mCam);
        else
            shadows[mCurrentScene][i]->SetConstantBuffer();

        if( mCurrentScene == CurrentScene::DARKNESS )
            shadows[mCurrentScene][i]->SetConstantBuffer(mCam);

        shadowBuff[mCurrentScene][i].lightPos = shadows[mCurrentScene][i]->lightPos;

        XMStoreFloat4x4( &shadowBuff[mCurrentScene][i].lightViewProj, shadows[mCurrentScene][i]->mShadowCam->ViewProj());

        sceneBuff.camPos = shadows[mCurrentScene][i]->mShadowCam->GetPosition();

        XMStoreFloat4x4(&sceneBuff.viewProj, shadows[mCurrentScene][i]->mShadowCam->ViewProj()); 

        mDevcon->VSSetConstantBuffers(0, 1, &sceneCBuffer);
        mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, &sceneBuff , 0, 0);

        mDevcon->OMSetRenderTargets(0, 0, shadows[mCurrentScene][i]->pShadowMapDepthView);

        for(int j = 0; j < scene[mCurrentScene]->mRenderables.size() ; j++)
        {
            scene[mCurrentScene]->mRenderables[j]->Depth();
        }
        for(int j = 0; j < scene[mCurrentScene]->mBlendRenderables.size() ; j++)
        {
            scene[mCurrentScene]->mBlendRenderables[j]->Depth();
        }
    }

    mDevcon->PSSetConstantBuffers(5, 1, &shadowCBuffer[mCurrentScene]);
    mDevcon->VSSetConstantBuffers(5, 1, &shadowCBuffer[mCurrentScene]);
    mDevcon->UpdateSubresource(shadowCBuffer[mCurrentScene], 0, 0, &shadowBuff[mCurrentScene][0] , 0, 0);
}

void RenderManager::SetShadowSRV(int loc)
{
    vector<ID3D11ShaderResourceView*> shadSRV;
    //int val = loc;
    //for( int i = 0; i < shadows[mCurrentScene].size(); i++ )
    //{
    //	val += shadows[mCurrentScene][0]->SetSRV(val);
    //}
    for( int i = 0; i < shadows[mCurrentScene].size(); i++ )
    {
        shadSRV.push_back( shadows[mCurrentScene][i]->pShadowMapSRView );
    }
    mDevcon->PSSetShaderResources(6, shadSRV.size(), &shadSRV[0]);
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

    for(int i = 0; i < (int)scene[mCurrentScene]->mRenderables.size() ; i++)
    {
        scene[mCurrentScene]->mRenderables[i]->RecompileShader();
    }
    for(int i = 0; i < (int)scene[mCurrentScene]->mBlendRenderables.size() ; i++)
    {
        scene[mCurrentScene]->mBlendRenderables[i]->RecompileShader();
    }
}

float RenderManager::Randomf(float low, float high)
{
    return low + (float)rand()/((float)RAND_MAX/(high-low));
}