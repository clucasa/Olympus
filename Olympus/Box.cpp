#include "Box.h"

Box::Box()
{
    
}

Box::Box(ID3D11DeviceContext *mDevcon, ID3D11Device *mDev, Apex* apex, float length, float width, float height) : 
    mDevcon(mDevcon), mDev(mDev), mApex(apex), mLength(length), mWidth(width), mHeight(height)
{
    GeometryGenerator* geoGen = new GeometryGenerator();
    mX = -20.0f;
    mY = 4.0f;
    mZ = 25.0f;
    cb = new cbuffs();

    CreateGeometry(geoGen);
    SetupBuffer();
    SetupPipeline();
}


void Box::SetupPipeline()
{
    // load and compile the two shaders
    ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("models.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("models.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);
    HRESULT hr;
    hr = mDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &mVS);
    if( FAILED(hr) )
        return ;
    mDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &mPS);
    if( FAILED(hr) )
        return ;

    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",	  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
        { "TEXNUM",   0, DXGI_FORMAT_R32_FLOAT,		  0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    mDev->CreateInputLayout(ied, 6, VS->GetBufferPointer(), VS->GetBufferSize(), &mLayout);

    hr = D3DX11CreateShaderResourceViewFromFile( mDev, "Media/Textures/Wood.png", NULL, NULL, &mTexture, NULL );
    if( FAILED( hr ) )
        return;

    hr = D3DX11CreateShaderResourceViewFromFile( mDev, "Media/Textures/BlankNormalMap.png", NULL, NULL, &mNmap, NULL );
    if( FAILED( hr ) )
        return;
}

void Box::CreateGeometry(GeometryGenerator *geoGen)
{
    GeometryGenerator::MeshData BoxData;			   // geometry for the sky box
    geoGen->CreateBox(mWidth, mHeight, mLength, BoxData);

    Vertex temp;
	XMVECTOR p;
	XMVECTOR n;
	XMVECTOR cross;
    for(size_t i = 0; i < BoxData.Vertices.size(); i++)
    {
        temp.Pos      = BoxData.Vertices[i].Position;
        temp.Normal   = BoxData.Vertices[i].Normal;
        temp.Tex      = BoxData.Vertices[i].TexC;
		temp.texNum   = 0;
        temp.Tangent  = BoxData.Vertices[i].TangentU;
        p = XMVectorSet( temp.Pos.x, temp.Pos.y, temp.Pos.z ,1.0 );
        n = XMVectorSet( temp.Normal.x, temp.Normal.y, temp.Normal.z ,1.0 );
        cross = XMVector3Cross(p, n);
        XMStoreFloat3(&temp.BiNormal, cross);
        vertices.push_back(temp);
    }

    for(size_t i = 0; i < BoxData.Indices.size(); i++)
    {
        indices.push_back(BoxData.Indices[i]);
    }

}



void Box::SetupBuffer()
{
    // create the vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
    bd.ByteWidth = sizeof(Vertex) * vertices.size();             // size is the VERTEX struct
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    mDev->CreateBuffer(&bd, NULL, &BoxVertBuffer);       // create the buffer


    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    mDevcon->Map(BoxVertBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, &vertices[0], sizeof(Vertex) * vertices.capacity());                 // copy the data
    mDevcon->Unmap(BoxVertBuffer, NULL);                                      // unmap the buffer

    // create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(UINT) * indices.size();    // 3 per triangle, 12 triangles
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;

    mDev->CreateBuffer(&bd, NULL, &BoxIndBuffer);

    mDevcon->Map(BoxIndBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);      // map the buffer
    memcpy(ms.pData, &indices[0], sizeof(UINT) * indices.size());                     // copy the data
    mDevcon->Unmap(BoxIndBuffer, NULL);


    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(cbuffs);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    HRESULT hr = mDev->CreateBuffer(&bd, NULL, &mConstBuffer);


    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(EnvironBuff);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &envCBuffer);
}

void Box::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{
    mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->PSSetShader(mPS, 0, 0);
    mDevcon->IASetInputLayout(mLayout);

     // select which vertex buffer to display
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    mDevcon->IASetVertexBuffers(0, 1, &BoxVertBuffer, &stride, &offset);

    mDevcon->IASetIndexBuffer(BoxIndBuffer, DXGI_FORMAT_R32_UINT, 0);

    // select which primtive type we are using
    mDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    XMMATRIX matTrans;
    
    //matTrans = XMMatrixTranslation(mCam->GetPosition().x, mCam->GetPosition().y, mCam->GetPosition().z);
    matTrans = XMMatrixTranslation(mX,mY,mZ);
    mDevcon->VSSetConstantBuffers(1, 1, &mConstBuffer);

	mDevcon->PSSetShaderResources(0, 1, &mTexture);
    mDevcon->PSSetShaderResources(1, 1, &mNmap);

    for(int i = 0; i < mWorldMats.size(); i++)
    {
        mDevcon->UpdateSubresource(mConstBuffer, 0, 0, &mWorldMats[i], 0, 0);
        mDevcon->DrawIndexed(indices.size(), 0, 0);
    }
}

void Box::Depth()
{
	mDevcon->VSSetShader(mVS, 0, 0);
    mDevcon->PSSetShader(NULL, 0, 0);
    mDevcon->IASetInputLayout(mLayout);

     // select which vertex buffer to display
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    mDevcon->IASetVertexBuffers(0, 1, &BoxVertBuffer, &stride, &offset);

    mDevcon->IASetIndexBuffer(BoxIndBuffer, DXGI_FORMAT_R32_UINT, 0);

    // select which primtive type we are using
    mDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    XMMATRIX matTrans;
    
    //matTrans = XMMatrixTranslation(mCam->GetPosition().x, mCam->GetPosition().y, mCam->GetPosition().z);
    matTrans = XMMatrixTranslation(mX,mY,mZ);
    mDevcon->VSSetConstantBuffers(1, 1, &mConstBuffer);
    for(int i = 0; i < mWorldMats.size(); i++)
    {
        mDevcon->UpdateSubresource(mConstBuffer, 0, 0, &mWorldMats[i], 0, 0);
        mDevcon->DrawIndexed(indices.size(), 0, 0);
    }
}

void Box::RecompileShader()
{
    // load and compile the two shaders
    ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("models.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("models.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);

    // encapsulate both shaders into shader objects
    mDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &mVS);
    
    mDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &mPS);
}

void Box::AddInstance(float x, float y, float z)
{
    XMMATRIX matTrans;
    
    matTrans = XMMatrixTranslation(x,y,z);
    XMFLOAT4X4 final;
    XMStoreFloat4x4(&final, matTrans);
    mWorldMats.push_back(final);
}

void Box::Update()
{

}