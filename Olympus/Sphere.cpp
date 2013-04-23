#include "Sphere.h"
#include "GeometryGenerator.h"
#include "System.h"

Sphere::Sphere()
{
	
}

Sphere::Sphere(ID3D11DeviceContext *mDevcon, ID3D11Device *mDev, GeometryGenerator *geoGen, Apex* apex, int radius, int slices, int stacks) : 
	mDevcon(mDevcon), mDev(mDev), radius(radius), slices(slices), stacks(stacks)
{
	cb = new cbuffs();
	cb->viewInvProj;
	cb->viewPrevProj;

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
    bd.ByteWidth = sizeof(cbuff);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &mConstBuffer);

}

void Sphere::getShit(ID3D11ShaderResourceView* mDynamicCubeMapSRVSphere)
{
	mDynamicCubeMap = mDynamicCubeMapSRVSphere;
}

void Sphere::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{
	if(renderType == 2){
		return;
	}

	mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->PSSetShader(mPS, 0, 0);
    mDevcon->IASetInputLayout(mLayout);


	cb->farZ = mCam->GetFarZ();
	cb->nearZ = mCam->GetNearZ();


	 // select which vertex buffer to display
    UINT stride = sizeof(PosNormalTexTan);
    UINT offset = 0;
    mDevcon->IASetVertexBuffers(0, 1, &SphereVertBuffer, &stride, &offset);

	mDevcon->IASetIndexBuffer(SphereIndBuffer, DXGI_FORMAT_R32_UINT, 0);

    // select which primtive type we are using
    mDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX matTrans;
	
	//matTrans = XMMatrixTranslation(mCam->GetPosition().x, mCam->GetPosition().y, mCam->GetPosition().z);
	matTrans = XMMatrixTranslation(0,4,0);

	XMFLOAT4X4 mWorldMat;
	XMStoreFloat4x4(&mWorldMat, matTrans);
	mDevcon->VSSetConstantBuffers(1, 1, &mConstBuffer);
	//mDevcon->PSSetConstantBuffers(0, 1, &sceneBuff);
	mDevcon->UpdateSubresource(mConstBuffer, 0, 0, &mWorldMat, 0, 0);

	mDevcon->PSSetShaderResources(0, 1, &mDynamicCubeMap);
	//mDevcon->PSSetShaderResources(0, 1, &mShaderResourceView);
	
	// set the new values for the constant buffer
	//mDevcon->UpdateSubresource(sceneBuff, 0, 0, mCam->ViewProj().m , 0, 0);
	//mDevcon->PSSetConstantBuffers(0, 1, &mConstBuffer);
	//mDevcon->UpdateSubresource(mConstBuffer, 0, 0, cb, 0, 0);

	

	 // draw the vertex buffer to the back buffer
	mDevcon->DrawIndexed(indices.size(), 0, 0);
//	mDevcon->PSSetConstantBuffers(0, 1, NULL);
}
