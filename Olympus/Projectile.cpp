#include "Projectile.h"

Projectile::Projectile( ID3D11Device* dev, ID3D11DeviceContext* devcon, Apex* apex ) :
	numBoxes(0), mApex(apex), mDev(dev),mDevcon(devcon)
{
	blockMaterial = mApex->getPhysics()->createMaterial(0.8f, 0.8f, 0.1f);    //static friction, dynamic friction, restitution
	if(!blockMaterial)
		return;
	
	SetupBoxMesh();

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( Vertex ) * vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &vertices[0];
	HRESULT hr = mDev->CreateBuffer( &bd, &InitData, &vertexBuffer );
	if( FAILED( hr ) )
		return;


    hr = D3DX11CreateShaderResourceViewFromFile( mDev, "Media/Textures/bricks.dds", NULL, NULL, &mTexture, NULL );
    if( FAILED( hr ) )
		return;

     hr = D3DX11CreateShaderResourceViewFromFile( mDev, "Media/Textures/bricks_nmap.dds", NULL, NULL, &mNmap, NULL );
    if( FAILED( hr ) )
		return;

	ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("models.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("models.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);
	
	hr = mDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &opVS);
    if( FAILED(hr) )
		return ;

	hr = mDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &opPS);
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

	mDev->CreateInputLayout(ied, 6, VS->GetBufferPointer(), VS->GetBufferSize(), &objLayout);


	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 64;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &worldCBuffer);
}

Projectile::~Projectile()
{

}

void Projectile::Fire(Camera *mCam, float speed)
{
	if(numBoxes < MAXBOXES)
	{
		PxVec3 look = PxVec3(mCam->GetLook().x,mCam->GetLook().y,mCam->GetLook().z);
		look.normalize();
		PxVec3 pos = PxVec3(mCam->GetPosition().x, mCam->GetPosition().y, mCam->GetPosition().z) + (look * 4.);
		PxReal density = 10.0f;
		
		PxTransform transform(pos, PxQuat::createIdentity());
		PxVec3 dimensions(.5,.5,.5);
		//PxSphereGeometry geometry(0.5);
		PxBoxGeometry geometry(dimensions);
		PxRigidDynamic* boxActor = PxCreateDynamic(*mApex->getPhysics(), transform, geometry, *blockMaterial, density);
		if (!boxActor)
			return;

		float vx = look.x * speed;
		float vy = look.y * speed;
		float vz = look.z * speed;

		boxActor->setLinearVelocity(PxVec3(vx,vy,vz));
		boxActor->setAngularDamping(0.95);

		PxRigidBodyExt::updateMassAndInertia(*boxActor, density);

		//CCD
		PxShape** shapes = new PxShape*[1];
		boxActor->getShapes(shapes, 1, 0);
		shapes[0]->setFlag(PxShapeFlag::eUSE_SWEPT_BOUNDS, true);
		delete [] shapes;

		mApex->getScene()->addActor(*boxActor);
		boxes.push_back(boxActor);

		numBoxes++;

		XMFLOAT4X4 final;
		XMMATRIX trans;

		trans = XMMatrixTranslation(pos.x,pos.y,pos.z);

		XMStoreFloat4x4(&final, trans );

		mWorldMats.push_back(final);
	}
}

void Projectile::Update()
{
	PxU32 nShapes = 0;

	for(int i = 0; i < numBoxes; i++)
	{
		if(boxes[i])
			nShapes = boxes[i]->getNbShapes();
		else	
			continue;

		PxShape** shapes = new PxShape*[nShapes];
 
		boxes[i]->getShapes(shapes, nShapes);     
		PxTransform pt = PxShapeExt::getGlobalPose(*shapes[0]);

		delete [] shapes;

        XMMATRIX world = XMLoadFloat4x4(&mWorldMats[i]) ;
        mApex->PxtoXMMatrix(pt, &world);

		XMStoreFloat4x4(&mWorldMats[i], world);
	}
}

void Projectile::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)	
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	mDevcon->VSSetShader(opVS, 0, 0);
	mDevcon->PSSetShader(opPS, 0, 0);
	
	mDevcon->VSSetConstantBuffers(1, 1, &worldCBuffer);


	mDevcon->IASetInputLayout(objLayout);
    mDevcon->PSSetShaderResources(0, 1, &mTexture );
    mDevcon->PSSetShaderResources(1, 1, &mNmap );
	mDevcon->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// select which primtive type we are using
	mDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for( int j = 0; j < mWorldMats.size(); j++)
	{
		mDevcon->UpdateSubresource(worldCBuffer, 0, 0, &mWorldMats[j], 0, 0);
		mDevcon->Draw( vertices.size(),0);
	}

}

void Projectile::Depth()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	mDevcon->VSSetShader(opVS, 0, 0);
	mDevcon->PSSetShader(NULL, 0, 0);
	
	mDevcon->VSSetConstantBuffers(1, 1, &worldCBuffer);


	mDevcon->IASetInputLayout(objLayout);
    mDevcon->PSSetShaderResources(0, 1, &mTexture );
    mDevcon->PSSetShaderResources(1, 1, &mNmap );
	mDevcon->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// select which primtive type we are using
	mDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for( int j = 0; j < mWorldMats.size(); j++)
	{
		mDevcon->UpdateSubresource(worldCBuffer, 0, 0, &mWorldMats[j], 0, 0);
		mDevcon->Draw( vertices.size(),0);
	}
}

void Projectile::RecompileShader()
{
	ID3D10Blob *VS, *PS;
	D3DX11CompileFromFile("models.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("models.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);
	HRESULT hr;
	hr = mDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &opVS);
    if( FAILED(hr) )
		return ;
	hr = mDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &opPS);
	if( FAILED(hr) )
		return ;
}

void Projectile::SetupBoxMesh()
{
	float side = 1.0f;
	float w2 = 0.5f * side;
	float h2 = 0.5f * side;
	float d2 = 0.5f * side;
    

    //Backface
	vertices.push_back(Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f));
	vertices.push_back(Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));

	vertices.push_back(Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
    vertices.push_back(Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f));

    //Frontface
    vertices.push_back(Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
	vertices.push_back(Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f));

	vertices.push_back(Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f));    
    vertices.push_back(Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f));
	vertices.push_back(Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f));

    //Left face
    vertices.push_back(Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f));
	vertices.push_back(Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f));

	vertices.push_back(Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f));    
    vertices.push_back(Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f));
	vertices.push_back(Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f));


    // right face
    vertices.push_back(Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f));
	vertices.push_back(Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));

	vertices.push_back(Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));   
	vertices.push_back(Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f)); 
    vertices.push_back(Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f));


    // top face
    vertices.push_back(Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f));
	vertices.push_back(Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));

    vertices.push_back(Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));
	vertices.push_back(Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f));

    // bottom face
    vertices.push_back(Vertex(-w2, -h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(+w2, -h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));
	vertices.push_back(Vertex(-w2, -h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f));

    vertices.push_back(Vertex(-w2, -h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
	vertices.push_back(Vertex(+w2, -h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f));
	vertices.push_back(Vertex(+w2, -h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));
}