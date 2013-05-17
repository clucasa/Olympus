#include "Grid.h"
#include "System.h"

Grid::Grid()
{
	
}

Grid::Grid(ID3D11DeviceContext *mDevcon, ID3D11Device *mDev, GeometryGenerator *geoGen) : 
	mDevcon(mDevcon), mDev(mDev)
{

	CreateGeometry(geoGen);
	SetupBuffer();
	SetupPipeline();
	SetupRenderTarget();
}

void Grid::SetupRenderTarget()
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
void Grid::SetupPipeline()
{
    // load and compile the two shaders
	ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("Grid.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("Grid.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);

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

void Grid::CreateGeometry(GeometryGenerator *geoGen)
{

	GeometryGenerator::MeshData GridData;			   // geometry for the sky box
	geoGen->CreateGrid(10, 10, 10, 10, GridData);


	for(size_t i = 0; i < GridData.Vertices.size(); i++)
    {
        vertices[i].Pos      = GridData.Vertices[i].Position;
        vertices[i].Normal   = GridData.Vertices[i].Normal;
        vertices[i].Tex      = GridData.Vertices[i].TexC;
        vertices[i].TangentU = GridData.Vertices[i].TangentU;
    }

	for(size_t i = 0; i < GridData.Indices.size(); i++)
	{
		indices[i] = GridData.Indices[i];
	}

}

void Grid::SetupBuffer()
{
	
    // create the vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
    bd.ByteWidth = sizeof(PosNormalTexTan) * 4;             // size is the VERTEX struct * 3
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    mDev->CreateBuffer(&bd, NULL, &GridVertBuffer);       // create the buffer


    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    mDevcon->Map(GridVertBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	memcpy(ms.pData, &vertices, sizeof(PosNormalTexTan) * 4);                 // copy the data
    mDevcon->Unmap(GridVertBuffer, NULL);                                      // unmap the buffer

	// create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(UINT) * 6;    // 3 per triangle, 12 triangles
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;

    mDev->CreateBuffer(&bd, NULL, &GridIndBuffer);

    mDevcon->Map(GridIndBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);      // map the buffer
	memcpy(ms.pData, &indices, sizeof(UINT) * 6);                     // copy the data
    mDevcon->Unmap(GridIndBuffer, NULL);


    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(cbuff);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &mConstBuffer);

}

void Grid::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{

	mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->PSSetShader(mPS, 0, 0);
    mDevcon->IASetInputLayout(mLayout);


	//cb->farZ = mCam->GetFarZ();
	//cb->nearZ = mCam->GetNearZ();

	 // select which vertex buffer to display
    UINT stride = sizeof(PosNormalTexTan);
    UINT offset = 0;
    mDevcon->IASetVertexBuffers(0, 1, &GridVertBuffer, &stride, &offset);

	mDevcon->IASetIndexBuffer(GridIndBuffer, DXGI_FORMAT_R32_UINT, 0);

    // select which primtive type we are using
    mDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX matTrans;
	
	//matTrans = XMMatrixTranslation(mCam->GetPosition().x, mCam->GetPosition().y, mCam->GetPosition().z);
	matTrans = XMMatrixTranslation(0,0,0);

	mDevcon->PSSetShaderResources(0, 1, &mShaderResourceView);
	

	 // draw the vertex buffer to the back buffer
    mDevcon->DrawIndexed(6, 0, 0);
}
