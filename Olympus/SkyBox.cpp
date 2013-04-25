#include "SkyBox.h"

SkyBox::SkyBox()
{
	
}

SkyBox::SkyBox(ID3D11DeviceContext *mDevcon, ID3D11Device *mDev, GeometryGenerator *geoGen) : 
	mDevcon(mDevcon), mDev(mDev)
{
	CreateGeometry(geoGen);
	SetupBuffer();
	SetupPipeline();
}
void SkyBox::SetupPipeline()
{
    // load and compile the two shaders
	ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("Skybox.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("Skybox.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);

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

void SkyBox::CreateGeometry(GeometryGenerator *geoGen)
{

	GeometryGenerator::MeshData skyBoxData;			   // geometry for the sky box
	geoGen->CreateBox(25.5f, 25.5f, 25.5f, skyBoxData);


	for(size_t i = 0; i < skyBoxData.Vertices.size(); i++)
    {
        vertices[i].Pos      = skyBoxData.Vertices[i].Position;
        vertices[i].Normal   = skyBoxData.Vertices[i].Normal;
        vertices[i].Tex      = skyBoxData.Vertices[i].TexC;
        vertices[i].TangentU = skyBoxData.Vertices[i].TangentU;
    }

	for(size_t i = 0; i < skyBoxData.Indices.size(); i++)
	{
		indices[i] = skyBoxData.Indices[35-i];
	}

}

void SkyBox::SetupBuffer()
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(mDev, "Media/Textures/mountains1024.dds", 0, 0, &mCubeMap, 0 );
	
    // create the vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
    bd.ByteWidth = sizeof(PosNormalTexTan) * 24;             // size is the VERTEX struct * 3
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    mDev->CreateBuffer(&bd, NULL, &skyBoxVertBuffer);       // create the buffer


    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    mDevcon->Map(skyBoxVertBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	memcpy(ms.pData, &vertices, sizeof(PosNormalTexTan) * 24);                 // copy the data
    mDevcon->Unmap(skyBoxVertBuffer, NULL);                                      // unmap the buffer

	// create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(UINT) * 36;    // 3 per triangle, 12 triangles
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;

    mDev->CreateBuffer(&bd, NULL, &skyBoxIndBuffer);

    mDevcon->Map(skyBoxIndBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);      // map the buffer
	memcpy(ms.pData, &indices, sizeof(UINT) * 36);                     // copy the data
    mDevcon->Unmap(skyBoxIndBuffer, NULL);


    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 64;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &mConstBuffer);

}

void SkyBox::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{
	mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->PSSetShader(mPS, 0, 0);
    mDevcon->IASetInputLayout(mLayout);

	 // select which vertex buffer to display
    UINT stride = sizeof(PosNormalTexTan);
    UINT offset = 0;
    mDevcon->IASetVertexBuffers(0, 1, &skyBoxVertBuffer, &stride, &offset);

	mDevcon->IASetIndexBuffer(skyBoxIndBuffer, DXGI_FORMAT_R32_UINT, 0);

    // select which primtive type we are using
    mDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX matTrans;
	
	matTrans = XMMatrixTranslation(mCam->GetPosition().x, mCam->GetPosition().y, mCam->GetPosition().z);
	//matTrans = XMMatrixTranslation(0,0,0);
	
	mDevcon->VSSetConstantBuffers(1, 1, &mConstBuffer);

	// set the new values for the constant buffer
	mDevcon->UpdateSubresource(mConstBuffer, 0, 0, matTrans.m, 0, 0);

	mDevcon->PSSetShaderResources(0, 1, &mCubeMap);

	 // draw the vertex buffer to the back buffer
    mDevcon->DrawIndexed(36, 0, 0);
}
