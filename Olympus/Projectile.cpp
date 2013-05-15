#include "Projectile.h"

Projectile::Projectile( ID3D11Device* dev, ID3D11DeviceContext* devcon, Apex* apex, int maxBoxes ) :
    numBoxes(0), curBox(0), mApex(apex), mDev(dev), mDevcon(devcon), mMaxBoxes(maxBoxes)
{
    blockMaterial = mApex->getPhysics()->createMaterial(0.1f, 0.1f, 0.0f);    //static friction, dynamic friction, restitution
    if(!blockMaterial)
        return;
    
    SetupSphereMesh();


    HRESULT hr = D3DX11CreateShaderResourceViewFromFile( mDev, "Media/Textures/bricks.dds", NULL, NULL, &mTexture, NULL );
    if( FAILED( hr ) )
        return;

     hr = D3DX11CreateShaderResourceViewFromFile( mDev, "Media/Textures/bricks_nmap.dds", NULL, NULL, &mNmap, NULL );
    if( FAILED( hr ) )
        return;

    ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("Sphere.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("Sphere.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);
    
    hr = mDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &opVS);
    if( FAILED(hr) )
        return ;

    hr = mDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &opPS);
    if( FAILED(hr) )
        return ;

    // create the input layout object
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    mDev->CreateInputLayout(ied, 4, VS->GetBufferPointer(), VS->GetBufferSize(), &objLayout);


    // create the vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
    bd.ByteWidth = sizeof(PosNormalTexTan) * vertices.size();             // size is the VERTEX struct
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    mDev->CreateBuffer(&bd, NULL, &vertexBuffer);       // create the buffer


    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    mDevcon->Map(vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, &vertices[0], sizeof(PosNormalTexTan) * vertices.capacity());                 // copy the data
    mDevcon->Unmap(vertexBuffer, NULL);                                      // unmap the buffer

    // create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(UINT) * indices.size();    // 3 per triangle, 12 triangles
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;

    mDev->CreateBuffer(&bd, NULL, &indexBuffer);

    mDevcon->Map(indexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);      // map the buffer
    memcpy(ms.pData, &indices[0], sizeof(UINT) * indices.size());                     // copy the data
    mDevcon->Unmap(indexBuffer, NULL);


    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(cbuffs);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    hr = mDev->CreateBuffer(&bd, NULL, &worldCBuffer);
}

Projectile::~Projectile()
{

}

void Projectile::Fire(Camera *mCam, float speed, vector<ApexCloth*> mCloths)
{
    if(mMaxBoxes <= 0)
        return;
    if(numBoxes < mMaxBoxes)
    {
        PxVec3 look = PxVec3(mCam->GetLook().x,mCam->GetLook().y,mCam->GetLook().z);
        look.normalize();
        PxVec3 pos = PxVec3(mCam->GetPosition().x, mCam->GetPosition().y, mCam->GetPosition().z) + (look * 4.);
        PxReal density = 100.0f;
        
        PxTransform transform(pos, PxQuat::createIdentity());
        PxVec3 dimensions(.5,.5,.5);
        PxSphereGeometry geometry(1.0f);
        //PxBoxGeometry geometry(dimensions);
        PxRigidDynamic* boxActor = PxCreateDynamic(*mApex->getPhysics(), transform, geometry, *blockMaterial, density);
        if (!boxActor)
            return;

        float vx = look.x * speed;
        float vy = look.y * speed;
        float vz = look.z * speed;

        boxActor->setLinearVelocity(PxVec3(vx,vy,vz));
        boxActor->setAngularDamping((PxReal)0.95f);

        PxRigidBodyExt::updateMassAndInertia(*boxActor, density);

        ////CCD
        //PxShape** shapes = new PxShape*[1];
        //boxActor->getShapes(shapes, 1, 0);
        //shapes[0]->setFlag(PxShapeFlag::eUSE_SWEPT_BOUNDS, true);
        //delete [] shapes;

        mApex->getScene()->addActor(*boxActor);
        boxes.push_back(boxActor);

        numBoxes++;
        curBox++;

        XMFLOAT4X4 final;
        XMMATRIX trans;

        trans = XMMatrixTranslation(pos.x,pos.y,pos.z);

        XMStoreFloat4x4(&final, trans );

        mWorldMats.push_back(final);
        if(mCloths.size() > 0)
            spheres.push_back(mCloths[0]->getClothingActor()->createCollisionSphere(pos, .5));
        
    }
    else
    {
        if(curBox >= mMaxBoxes)
            curBox = 0;
        
        PxVec3 look = PxVec3(mCam->GetLook().x,mCam->GetLook().y,mCam->GetLook().z);
        look.normalize();
        PxVec3 pos = PxVec3(mCam->GetPosition().x, mCam->GetPosition().y, mCam->GetPosition().z) + (look * 6.);
        PxTransform transform(pos, PxQuat::createIdentity());
        
        float vx = look.x * speed;
        float vy = look.y * speed;
        float vz = look.z * speed;

        boxes[curBox]->setGlobalPose(transform);
        PxRigidDynamic* boxDynamic = static_cast<PxRigidDynamic*>(boxes[curBox]);
        boxDynamic->setLinearVelocity(PxVec3(vx,vy,vz));
        boxDynamic->setAngularVelocity(PxVec3(0,0,0));
        boxDynamic->setGlobalPose(transform);

        if(mCloths.size() > 0)
        {
            physx::apex::NxClothingSphere* sphere = spheres[curBox];
            sphere->setPosition(pos);
            
        }

        curBox++;
    }
}

void Projectile::Clear()
{
    curBox = 0;
    for(int i = 0; i < numBoxes; i++)
    {
        boxes[i]->release();
        if(spheres.size() > i)
            spheres[i]->release();
    }
    spheres.clear();
    mWorldMats.clear();
    boxes.clear();
    numBoxes = 0;
}

void Projectile::Update()
{
    if(mMaxBoxes <= 0)
        return;
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

        if(spheres.size() > 0)
        {
            physx::apex::NxClothingSphere* sphere = spheres[i];

            PxVec3 pos = pt.p;
            sphere->setPosition(pos);
        }
    }
}

void Projectile::Render(ID3D11Buffer *sceneBuff, Camera *mCam, int renderType)	
{
    UINT stride = sizeof(PosNormalTexTan);
    UINT offset = 0;

    mDevcon->VSSetShader(opVS, 0, 0);
    mDevcon->PSSetShader(opPS, 0, 0);
    
    mDevcon->VSSetConstantBuffers(1, 1, &worldCBuffer);
    
    mDevcon->IASetInputLayout(objLayout);
    mDevcon->PSSetShaderResources(0, 1, &mTexture );
    mDevcon->PSSetShaderResources(1, 1, &mNmap );
    mDevcon->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    mDevcon->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // select which primtive type we are using
    mDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for( int j = 0; j < (int)mWorldMats.size(); j++)
    {
        mDevcon->UpdateSubresource(worldCBuffer, 0, 0, &mWorldMats[j], 0, 0);
        mDevcon->DrawIndexed(indices.size(), 0, 0);//mDevcon->Draw( vertices.size(),0);
    }
}

void Projectile::Depth()
{
    UINT stride = sizeof(PosNormalTexTan);
    UINT offset = 0;

    mDevcon->VSSetShader(opVS, 0, 0);
    mDevcon->PSSetShader(NULL, 0, 0);
    
    mDevcon->VSSetConstantBuffers(1, 1, &worldCBuffer);
    
    mDevcon->IASetInputLayout(objLayout);
    mDevcon->PSSetShaderResources(0, 1, &mTexture );
    mDevcon->PSSetShaderResources(1, 1, &mNmap );
    mDevcon->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    mDevcon->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // select which primtive type we are using
    mDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for( int j = 0; j < (int)mWorldMats.size(); j++)
    {
        mDevcon->UpdateSubresource(worldCBuffer, 0, 0, &mWorldMats[j], 0, 0);
        mDevcon->DrawIndexed(indices.size(), 0, 0);//mDevcon->Draw( vertices.size(),0);
    }
}

void Projectile::RecompileShader()
{
    ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("Sphere.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("Sphere.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);
    HRESULT hr;
    hr = mDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &opVS);
    if( FAILED(hr) )
        return ;
    hr = mDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &opPS);
    if( FAILED(hr) )
        return ;
}

void Projectile::SetupSphereMesh()
{
    float radius = 1.0f;
    GeometryGenerator* geoGen = new GeometryGenerator();
    GeometryGenerator::MeshData SphereData;
    geoGen->CreateSphere(radius, 15, 15, SphereData);

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