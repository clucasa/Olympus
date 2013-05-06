#include "Sphere.h"
#include "GeometryGenerator.h"
#include "System.h"

Sphere::Sphere()
{
	
}

Sphere::Sphere(ID3D11DeviceContext *mDevcon, ID3D11Device *mDev, GeometryGenerator *geoGen, Apex* apex, int radius, int slices, int stacks) : 
	mDevcon(mDevcon), mDev(mDev), radius(radius), slices(slices), stacks(stacks), reflective(false)
{
	mX = -20.0f;
	mY = 4.0f;
	mZ = 25.0f;
	cb = new cbuffs();

	CreateGeometry(geoGen);
	SetupBuffer();
	SetupPipeline();
	SetupRenderTarget();
}

void Sphere::SetupRenderTarget()
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;


	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = SCREEN_WIDTH;
	textureDesc.Height = SCREEN_HEIGHT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	result = mDev->CreateTexture2D(&textureDesc, NULL, &mTargetTexture);

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = mDev->CreateRenderTargetView(mTargetTexture, &renderTargetViewDesc, &mTargetView);


	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = mDev->CreateShaderResourceView(mTargetTexture, &shaderResourceViewDesc, &mShaderResourceView);

}

void Sphere::SetupPipeline()
{
    // load and compile the two shaders
	ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("Sphere.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("Sphere.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);

    // encapsulate both shaders into shader objects
    mDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &mVS);
    

    mDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &mPS);
    
    
    // create the input layout object
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    mDev->CreateInputLayout(ied, 4, VS->GetBufferPointer(), VS->GetBufferSize(), &mLayout);
   
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(mDev, "Media/Textures/cloudymountains2048new.dds", 0, 0, &mDynamicCubeMapSRVSphere, 0 );
}

void Sphere::CreateGeometry(GeometryGenerator *geoGen)
{

	GeometryGenerator::MeshData SphereData;			   // geometry for the sky box
	geoGen->CreateSphere(radius, slices, stacks, SphereData);

	PosNormalTexTan temp;

	for(size_t i = 0; i < SphereData.Vertices.size(); i++)
    {
        temp.Pos      = SphereData.Vertices[i].Position;
        temp.Normal   = SphereData.Vertices[i].Normal;
        temp.Tex      = SphereData.Vertices[i].TexC;
        temp.TangentU = SphereData.Vertices[i].TangentU;

		vertices.push_back(temp);
    }


	for(size_t i = 0; i < SphereData.Indices.size(); i++)
	{
		indices.push_back(SphereData.Indices[i]);
	}

}

void Sphere::SetupBuffer()
{
	
    // create the vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(PosNormalTexTan) * vertices.size();             // size is the VERTEX struct
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    mDev->CreateBuffer(&bd, NULL, &SphereVertBuffer);       // create the buffer


    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    mDevcon->Map(SphereVertBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	memcpy(ms.pData, &vertices[0], sizeof(PosNormalTexTan) * vertices.capacity());                 // copy the data
    mDevcon->Unmap(SphereVertBuffer, NULL);                                      // unmap the buffer

	// create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(UINT) * indices.size();    // 3 per triangle, 12 triangles
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;

    mDev->CreateBuffer(&bd, NULL, &SphereIndBuffer);

    mDevcon->Map(SphereIndBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);      // map the buffer
	memcpy(ms.pData, &indices[0], sizeof(UINT) * indices.size());                     // copy the data
    mDevcon->Unmap(SphereIndBuffer, NULL);


    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(cbuffs);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &mConstBuffer);


	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(EnvironBuff);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &envCBuffer);
}

void Sphere::SetupReflective(vector<Renderable*> *renderables, Renderable *skyBox,
						ScreenQuad *screenQuad, ID3D11DepthStencilView *zbuff,
						D3D11_VIEWPORT *screenViewport)
{
	reflective = true;

	mRenderables = renderables;
	mSkyBox = skyBox;
	mScreen = screenQuad;
	mZbuffer = zbuff;
	mScreenViewport = screenViewport;
	
	BuildDynamicCubeMapViewsSphere();
	BuildCubeFaceCamera(mX, mY, mZ);
}

void Sphere::BuildCubeFaceCamera(float x, float y, float z)
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

void Sphere::BuildDynamicCubeMapViewsSphere()
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

void Sphere::DynamicCubeMapRender(ID3D11Buffer *sceneBuff, int renderType, Camera cam)
{
	for(int i = 0; i < mRenderables->size() ; i++)
	{
		mRenderables[0][i]->Render(sceneBuff, &cam, renderTargets::environment);
	}
}

void Sphere::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{
	
	if(reflective)
	{
		if(renderType == renderTargets::environment)
		{
			return;
		}

		 // Sphere position
		mDevcon->RSSetViewports(1, &mCubeMapViewport);
		for(int i = 0; i < 6; ++i) // for mirror, just do (int i = 0; i < 1; ++i) for 1 camera mapped to mirror surface
		{
			// Clear cube map face and depth buffer.
			mDevcon->ClearRenderTargetView(mDynamicCubeMapRTVSphere[i], reinterpret_cast<const float*>(&Colors::Blue));
			mDevcon->ClearDepthStencilView(mDynamicCubeMapDSVSphere, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

			// Bind cube map face as render target.
			mDevcon->OMSetRenderTargets(1, &mDynamicCubeMapRTVSphere[i], 0);

			XMStoreFloat4x4(&sphereBuff.viewProj, mCubeMapCamera[i].ViewProj());
			sphereBuff.camPos = mCubeMapCamera[i].GetPosition();
			sphereBuff.ambientOn = 1;
			sphereBuff.diffuseOn = 1;
			sphereBuff.dirLightOn = 1;
			sphereBuff.textures = 1;
			mDevcon->UpdateSubresource(sceneBuff, 0, 0, &sphereBuff , 0, 0);

			// Draw the scene with the exception of the center sphere to this cube map face
			mSkyBox->Render(sceneBuff, &mCubeMapCamera[i], 0);
	

			// Bind cube map face as render target.
			mDevcon->OMSetRenderTargets(1, &mDynamicCubeMapRTVSphere[i], mDynamicCubeMapDSVSphere);

			DynamicCubeMapRender(sceneBuff, 0, mCubeMapCamera[i]);
		}
		mDevcon->GenerateMips(mDynamicCubeMapSRVSphere);

		mDevcon->OMSetRenderTargets(1, &mScreen->mTargetView/*mBackbuffer*/, mZbuffer);
		mDevcon->RSSetViewports(1, mScreenViewport);
	}

	mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->PSSetShader(mPS, 0, 0);
    mDevcon->IASetInputLayout(mLayout);



	 // select which vertex buffer to display
    UINT stride = sizeof(PosNormalTexTan);
    UINT offset = 0;
    mDevcon->IASetVertexBuffers(0, 1, &SphereVertBuffer, &stride, &offset);

	mDevcon->IASetIndexBuffer(SphereIndBuffer, DXGI_FORMAT_R32_UINT, 0);

    // select which primtive type we are using
    mDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX matTrans;
	
	//matTrans = XMMatrixTranslation(mCam->GetPosition().x, mCam->GetPosition().y, mCam->GetPosition().z);
	matTrans = XMMatrixTranslation(mX,mY,mZ);

	XMFLOAT4X4 mWorldMat[2];
	XMStoreFloat4x4(&mWorldMat[0], matTrans);

	XMStoreFloat4x4(&mWorldMat[1], XMMatrixInverse(&XMMatrixDeterminant(matTrans), matTrans));

	mDevcon->VSSetConstantBuffers(1, 1, &mConstBuffer);
	//mDevcon->PSSetConstantBuffers(0, 1, &sceneBuff);
	mDevcon->UpdateSubresource(mConstBuffer, 0, 0, &mWorldMat, 0, 0);
	//mDevcon->PSSetShaderResources(0, 1, NULL);
	if(reflective)
	{
		XMStoreFloat4x4(&sphereBuff.viewProj, mCam->ViewProj());
		sphereBuff.camPos = mCam->GetPosition();
		mDevcon->UpdateSubresource(sceneBuff, 0, 0, &sphereBuff, 0, 0);
	}
		mDevcon->PSSetShaderResources(0, 1, &mDynamicCubeMapSRVSphere);
	
	// else texture?

	//mDevcon->PSSetShaderResources(0, 1, &mShaderResourceView);
	
	// set the new values for the constant buffer
	//mDevcon->UpdateSubresource(sceneBuff, 0, 0, mCam->ViewProj().m , 0, 0);
	//mDevcon->PSSetConstantBuffers(0, 1, &mConstBuffer);
	//mDevcon->UpdateSubresource(mConstBuffer, 0, 0, cb, 0, 0);

	

	 // draw the vertex buffer to the back buffer
	mDevcon->DrawIndexed(indices.size(), 0, 0);
//	mDevcon->PSSetConstantBuffers(0, 1, NULL);
}

void Sphere::RecompileShader()
{
	// load and compile the two shaders
	ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("Sphere.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("Sphere.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);

    // encapsulate both shaders into shader objects
    mDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &mVS);
    
    mDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &mPS);
}

void Sphere::MoveTo(float x, float y, float z)
{
	mX = x;
	mY = y;
	mZ = z;

	if(reflective)
	{
		BuildCubeFaceCamera(mX, mY, mZ);
	}
}

void Sphere::IsItReflective(bool isReflective)
{
	reflective = isReflective;
}