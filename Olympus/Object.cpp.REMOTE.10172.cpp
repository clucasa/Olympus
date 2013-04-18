#include "Object.h"

Object::Object()
{

}
//---------------------------------------------------------------------------------------
// Character Loader here
//---------------------------------------------------------------------------------------
void Object::objLoad( char* filename, vector<LPSTR> *textures, vector<LPSTR> *NormTextures, ID3D11Device* devv, ID3D11DeviceContext *devcon, Apex* apex )
{
	dev1 = devv;
	devcon1 = devcon;
	vector<XMFLOAT3> vertexices;
	vector<XMFLOAT3> normexices;
	vector<XMFLOAT3> texexices;
	vector<WORD> indexices;
	vector<int> texNo;
	int tempTexCount;

	vector<Vertex> verts;

	numMeshes = Import( filename, &vertexes );
	ID3D11Buffer* tempVB;

	for( int i = 0; i < numMeshes; i++ )
	{
		int numVerts = vertexes[i].size();
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

		PxVec3* vertices = new PxVec3[numVerts];
		for(int j = 0; j < numVerts; j++)
		{
			vertices[j].x = vertexes[i][j].Pos.x;
			vertices[j].y = vertexes[i][j].Pos.y;
			vertices[j].z = vertexes[i][j].Pos.z;
		}

		apex->LoadTriangleMesh(numVerts, vertices,5.0f);
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

	for( int i = 0; i < textures->size(); i++ )
	{
		hr1 = D3DX11CreateShaderResourceViewFromFile( dev1, textures[0][i], NULL, NULL, &g_pTextureRV1, NULL );
		texArray.push_back( g_pTextureRV1 );
		hr1 = D3DX11CreateShaderResourceViewFromFile( dev1, NormTextures[0][i], NULL, NULL, &g_pTextureRV1, NULL );
		NormArray.push_back( g_pTextureRV1 );
	}
	
	//numMeshes = 1;//tempTexCount;
	alpha = 0;
	textures->resize(0);
	NormTextures->resize(0);
//	return;

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
    bd.ByteWidth = 64 * 2;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    dev1->CreateBuffer(&bd, NULL, &worldCBuffer);
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

void Object::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)
{
	    UINT stride = sizeof(Vertex);
        UINT offset = 0;

		devcon1->VSSetShader(opVS, 0, 0);
		devcon1->PSSetShader(opPS, 0, 0);

		XMStoreFloat4x4(&mWorldMat[0], XMMatrixScaling(5.0f,5.0f,5.0f));

		XMStoreFloat4x4(&mWorldMat[1], XMMatrixInverse(&XMMatrixDeterminant(XMLoadFloat4x4(&mWorldMat[0])), XMLoadFloat4x4(&mWorldMat[0])));

		devcon1->VSSetConstantBuffers(1, 1, &worldCBuffer);

		devcon1->UpdateSubresource(worldCBuffer, 0, 0, &mWorldMat, 0, 0);

		for( int i = 0; i < numMeshes; i++ )
		{
			devcon1->IASetInputLayout(objLayout);
			devcon1->PSSetShaderResources(0, 1, &texArray[i] );
			devcon1->PSSetShaderResources(1, 1, &NormArray[i] );

			devcon1->IASetVertexBuffers(0, 1, &vertexBuffer[i], &stride, &offset);

			//devcon->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);

			// select which primtive type we are using
			devcon1->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// draw the vertex buffer to the back buffer
		   // devcon->DrawIndexed(36, 0, 0);

			devcon1->Draw( vertexes[i].size(),0);
		}
}
