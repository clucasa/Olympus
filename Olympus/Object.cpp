#include "Object.h"

Object::Object(AssetManager* assetManager) : mAssetManager(assetManager)
{

}
//---------------------------------------------------------------------------------------
// Character Loader here
//---------------------------------------------------------------------------------------
void Object::objLoad( char* filename, vector<LPCSTR> *textures, vector<LPCSTR> *NormTextures, vector<float> TextureScales, ID3D11Device* devv, ID3D11DeviceContext *devcon, Apex* apex )
{
    mApex = apex;
    mScale = 0;
    dev1 = devv;
    devcon1 = devcon;
    vector<XMFLOAT3> vertexices;
    vector<XMFLOAT3> normexices;
    vector<XMFLOAT3> texexices;
    vector<WORD> indexices;
    vector<int> texNo;

    vector<Vertex> verts;

    //numMeshes = Import( filename, &vertexes );
    vertexes = mAssetManager->RequestModel( filename, numMeshes );
    ID3D11Buffer* tempVB;

    

    for( int i = 0; i < numMeshes; i++ )
    {
        int numVerts = vertexes[i].size();
        for(int j = 0; j < numVerts; j++)
        {
            vertexes[i][j].Tex.x *= TextureScales[i];
            vertexes[i][j].Tex.y *= TextureScales[i];
        }
        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof( Vertex ) * numVerts;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = &vertexes[i][0];
        hr1 = dev1->CreateBuffer( &bd, &InitData, &tempVB );
        if( FAILED( hr1 ) )
            return;

        vertexBuffer.push_back( tempVB );
    }
    // Save the Vertex Buffer for easy access
    //vertexBuffer = pVBuffer1;

    //// Load the Indice Buffer
    //bd.Usage = D3D10_USAGE_DEFAULT;
    //bd.ByteWidth = sizeof( int ) * indices_vec.size();
    //bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    //bd.CPUAccessFlags = 0;
    //bd.MiscFlags = 0;
    //InitData.pSysMem = &indices_vec[0];
    //hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
    //if( FAILED( hr ) )
    //	return;

    // Save the Index Buffer for easy access
    //	indexBuffer = g_pIndexBuffer;

    for( int i = 0; i < (int)textures->size(); i++ )
    {
        //hr1 = D3DX11CreateShaderResourceViewFromFile( dev1, textures[0][i], NULL, NULL, &g_pTextureRV1, NULL );
        g_pTextureRV1 = mAssetManager->RequestTexture(textures[0][i]);
        texArray.push_back( g_pTextureRV1 );
    }
    for( int i = 0; i < (int)NormTextures->size(); i++ )
    {
        //hr1 = D3DX11CreateShaderResourceViewFromFile( dev1, NormTextures[0][i], NULL, NULL, &g_pTextureRV1, NULL );
        g_pTextureRV1 = mAssetManager->RequestTexture(NormTextures[0][i]);
        NormArray.push_back( g_pTextureRV1 );
    }
    
    //numMeshes = 1;//tempTexCount;
    alpha = 0;
    textures->resize(0);
    NormTextures->resize(0);
//	return;

    ID3D10Blob *VS, *PS;
    /*D3DX11CompileFromFile("models.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("models.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);

    HRESULT hr;
    hr = dev1->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &opVS);
    if( FAILED(hr) )
        return;

    dev1->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &opPS);
    if( FAILED(hr) )
        return;*/

	Asset* vertS = mAssetManager->RequestVShader("models.hlsl");
	opVS = vertS->vertexShader;
	VS = vertS->VS;

	Asset* pixS = mAssetManager->RequestPShader("models.hlsl");
	opPS = pixS->pixelShader;
	PS = pixS->PS;

	HRESULT hr;
    D3DX11CompileFromFile("models.hlsl", 0, 0, "PSAplhaShadow", "ps_5_0", 0, 0, 0, &PS, 0, 0);
    dev1->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &opPSAlpha);
    
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",	  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
        { "TEXNUM",   0, DXGI_FORMAT_R32_FLOAT,		  0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    devv->CreateInputLayout(ied, 6, VS->GetBufferPointer(), VS->GetBufferSize(), &objLayout);

    oVS = VS;
    oPS = PS;


    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(cbuffs);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    hr = dev1->CreateBuffer(&bd, NULL, &worldCBuffer);
	if( FAILED(hr) )
        return ;
}

void Object::renderO( ID3D11DeviceContext * devcon)
{
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        
        for( int i = 0; i < numMeshes; i++ )
        {
            devcon->IASetInputLayout(objLayout);
            devcon->PSSetShaderResources(0, 1, &texArray[i] );
            devcon->PSSetShaderResources(1, 1, &NormArray[i] );

            devcon->IASetVertexBuffers(0, 1, &vertexBuffer[i], &stride, &offset);

            //devcon->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);

            // select which primtive type we are using
            devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // draw the vertex buffer to the back buffer
           // devcon->DrawIndexed(36, 0, 0);

            devcon->Draw( vertexes[i].size(),0);
        }
}

void Object::AddInstance(ObjectInfo info)
{
    XMFLOAT4X4 final;
    XMMATRIX scale, trans, rot;
    mScale = info.sx;
    scale = XMMatrixScaling(info.sx,info.sy,info.sz);

    trans = XMMatrixTranslation(info.x,info.y,info.z);

    rot = XMMatrixRotationY(info.ry);
    rot *= XMMatrixRotationX(info.rx);
    rot *= XMMatrixRotationZ(info.rz);

    XMStoreFloat4x4(&final, XMMatrixMultiply(scale,  XMMatrixMultiply(rot, trans) ) );

    mWorldMats.push_back(final);
    for( int i = 0; i < numMeshes; i++ )
    {
        int numVerts = vertexes[i].size();
        PxVec3* vertices = new PxVec3[numVerts];
        for(int j = 0; j < numVerts; j++)
        {
            vertices[j].x = vertexes[i][j].Pos.x;
            vertices[j].y = vertexes[i][j].Pos.y;
            vertices[j].z = vertexes[i][j].Pos.z;
        }
        if(info.materials[i].dynamicOn)
        {
            mApex->LoadDynamicTriangleMesh(numVerts, vertices, info);
        }
        else
        {
            mApex->LoadTriangleMesh(numVerts, vertices, info);
        }
    }

    // For this instance, have these materials for the # of meshes
    materials.push_back( info.materials );
}


void Object::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{
    XMMATRIX tempMat;
    

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    devcon1->VSSetShader(opVS, 0, 0);
    devcon1->PSSetShader(opPS, 0, 0);
    
    devcon1->VSSetConstantBuffers(1, 1, &worldCBuffer);
    devcon1->PSSetConstantBuffers(1, 1, &worldCBuffer);

    for( int i = 0; i < numMeshes; i++ )
    {
        devcon1->IASetInputLayout(objLayout);
        devcon1->PSSetShaderResources(0, 1, &texArray[i] );
        devcon1->PSSetShaderResources(1, 1, &NormArray[i] );

        devcon1->IASetVertexBuffers(0, 1, &vertexBuffer[i], &stride, &offset);

        // select which primtive type we are using
        devcon1->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        for( int j = 0; j < (int)mWorldMats.size(); j++)
        {
            cb.material = materials[j][i]; // Per mesh

            tempMat = XMLoadFloat4x4(&mWorldMats[j]);
            tempMat = XMMatrixInverse(&XMMatrixDeterminant(tempMat), tempMat);
            
            
            cb.matWorld = mWorldMats[j];
            XMStoreFloat4x4(&cb.matWorldInvTrans, tempMat);

            devcon1->UpdateSubresource(worldCBuffer, 0, 0, &cb, 0, 0);
            devcon1->Draw( vertexes[i].size(),0);
        }
    }
}

void Object::Depth()
{
    XMMATRIX tempMat;
    

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    devcon1->VSSetShader(opVS, 0, 0);
    devcon1->PSSetShader(NULL, 0, 0);
    
    devcon1->VSSetConstantBuffers(1, 1, &worldCBuffer);
    devcon1->PSSetConstantBuffers(1, 1, &worldCBuffer);

    for( int i = 0; i < numMeshes; i++ )
    {
        devcon1->IASetInputLayout(objLayout);
        devcon1->PSSetShaderResources(0, 1, &texArray[i] );
        devcon1->PSSetShaderResources(1, 1, &NormArray[i] );

        devcon1->IASetVertexBuffers(0, 1, &vertexBuffer[i], &stride, &offset);

        // select which primtive type we are using
        devcon1->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        

        for( int j = 0; j < (int)mWorldMats.size(); j++)
        {
            cb.material = materials[j][i]; // Per mesh

			if( materials[j][i].alphaKillOn == 1 )
			{
				    devcon1->VSSetShader(opVS, 0, 0);
					devcon1->PSSetShader(opPSAlpha, 0, 0);
			}
            tempMat = XMLoadFloat4x4(&mWorldMats[j]);
            tempMat = XMMatrixInverse(&XMMatrixDeterminant(tempMat), tempMat);
            
            cb.matWorld = mWorldMats[j];
            XMStoreFloat4x4(&cb.matWorldInvTrans, tempMat);

            devcon1->UpdateSubresource(worldCBuffer, 0, 0, &cb, 0, 0);
            devcon1->Draw( vertexes[i].size(),0);
        }
    }
}

void Object::RecompileShader()
{

    ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("models.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("models.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);
    HRESULT hr;
    hr = dev1->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &opVS);
    if( FAILED(hr) )
        return ;
    dev1->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &opPS);
    if( FAILED(hr) )
        return ;
}