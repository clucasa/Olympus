#include "GroundPlane.h"
#include "System.h"

GroundPlane::GroundPlane()
{
	
}

GroundPlane::GroundPlane(ID3D11DeviceContext *mDevcon, ID3D11Device *mDev, GeometryGenerator *geoGen, int planeSize, int increment) : 
	mDevcon(mDevcon), mDev(mDev), size(planeSize), inc(increment)
{
	cb = new cbuff();
	cb->viewInvProj;
	cb->viewPrevProj;

	CreateGeometry(geoGen);
	SetupBuffer();
	SetupPipeline();
	SetupRenderTarget();
}

void GroundPlane::SetupRenderTarget()
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
void GroundPlane::SetupPipeline()
{
    // load and compile the two shaders
	ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("GroundPlane.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("GroundPlane.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);

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
   
}

void GroundPlane::CreateGeometry(GeometryGenerator *geoGen)
{

	GeometryGenerator::MeshData GroundPlaneData;			   // geometry for the sky box
	geoGen->CreateGrid(size, size, inc, inc, GroundPlaneData);

	PosNormalTexTan temp;
	for(size_t i = 0; i < GroundPlaneData.Vertices.size(); i++)
    {
		temp.Pos		= GroundPlaneData.Vertices[i].Position;
        temp.Normal		= GroundPlaneData.Vertices[i].Normal;
		temp.Tex		= GroundPlaneData.Vertices[i].TexC;
        temp.TangentU	= GroundPlaneData.Vertices[i].TangentU;
		vertices.push_back( temp );
    }

	for(size_t i = 0; i < GroundPlaneData.Indices.size(); i++)
	{
		indices.push_back( GroundPlaneData.Indices[i] );
	}

}

void GroundPlane::SetupBuffer()
{
	
    // create the vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
    bd.ByteWidth = sizeof(PosNormalTexTan) * vertices.size();             // size is the VERTEX struct * 3
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    mDev->CreateBuffer(&bd, NULL, &GroundPlaneVertBuffer);       // create the buffer


    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    mDevcon->Map(GroundPlaneVertBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	memcpy(ms.pData, &vertices[0], sizeof(PosNormalTexTan) * vertices.size());                 // copy the data
    mDevcon->Unmap(GroundPlaneVertBuffer, NULL);                                      // unmap the buffer

	// create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(UINT) * indices.size();    // 3 per triangle, 12 triangles
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;

    mDev->CreateBuffer(&bd, NULL, &GroundPlaneIndBuffer);

    mDevcon->Map(GroundPlaneIndBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);      // map the buffer
	memcpy(ms.pData, &indices[0], sizeof(UINT) * indices.size());                     // copy the data
    mDevcon->Unmap(GroundPlaneIndBuffer, NULL);


    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(cbuff);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &mConstBuffer);

}

void GroundPlane::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{

	mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->PSSetShader(mPS, 0, 0);
    mDevcon->IASetInputLayout(mLayout);


	cb->farZ = mCam->GetFarZ();
	cb->nearZ = mCam->GetNearZ();

	XMStoreFloat4x4( &cb->viewInvProj, mCam->ViewProj() );
	//XMStoreFloat4x4(&cb->viewInvProj, XMMatrixInverse( &XMMatrixDeterminant( XMLoadFloat4x4( &cb->viewInvProj ) ), XMLoadFloat4x4( &cb->viewInvProj ) ) );

//	XMStoreFloat4x4( &cb->viewInvProj, XMMatrixTranspose( XMLoadFloat4x4( &cb->viewInvProj ) ) );
//	XMStoreFloat4x4( &cb->viewPrevProj, XMMatrixTranspose( XMLoadFloat4x4( &cb->viewPrevProj ) ) );

	 // select which vertex buffer to display
    UINT stride = sizeof(PosNormalTexTan);
    UINT offset = 0;
    mDevcon->IASetVertexBuffers(0, 1, &GroundPlaneVertBuffer, &stride, &offset);

	mDevcon->IASetIndexBuffer(GroundPlaneIndBuffer, DXGI_FORMAT_R32_UINT, 0);

    // select which primtive type we are using
    mDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX matTrans;
	
	//matTrans = XMMatrixTranslation(mCam->GetPosition().x, mCam->GetPosition().y, mCam->GetPosition().z);
	matTrans = XMMatrixTranslation(0,0,0);

	mDevcon->PSSetShaderResources(0, 1, &mShaderResourceView);
	
	// set the new values for the constant buffer
	//mDevcon->UpdateSubresource(sceneBuff, 0, 0, mCam->ViewProj().m , 0, 0);
	mDevcon->PSSetConstantBuffers(0, 1, &mConstBuffer);
	mDevcon->UpdateSubresource(mConstBuffer, 0, 0, cb, 0, 0);

	 // draw the vertex buffer to the back buffer
    mDevcon->DrawIndexed(indices.size(), 0, 0);

	XMStoreFloat4x4( &cb->viewPrevProj, mCam->ViewProj() );
}