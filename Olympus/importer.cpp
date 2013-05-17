//----------------------------------------------------------------------------------
// Hacked and updated version of FbxView found on conorgraphics 
// http://www.conorgraphics.com/?p=346
// Updated by Michael Gray
// No promises this works, not responsible if your machine goes up in a roaring
// ball of fire or you turn into a gypsie.
//----------------------------------------------------------------------------------
#include "importer.h"

using namespace std;

FbxScene*	lFbxScene;
FbxManager* lSdkManager;
string		lFileName;

int size = 0;

void LoadScene(char* filename);
void ProcessScene();
void ProcessNode( FbxNode* node, int attributeType);
void ProcessMesh( FbxNode* node );
FbxVector2 GetTexCoords( FbxMesh* mesh, int layerIndex, int polygonIndex, int polygonVertexIndex, int vertexIndex );


vector<vector<Vertex>>* Vertices;
int TexCount = 0;

int meshMatMax = 0;


// We just use the main function to get the party started. From here we will call everything in a straight line hopefully
int Import( char* filename, vector<vector<Vertex>>* vert )
{
    size = 0;
    TexCount = 0;

    Vertices = vert;

    LoadScene( filename );

    cout << "Finished" << endl;
    int a;
    cin >> a;

    return TexCount;//Vertices->size();
}


// Get the scene initialization running
void LoadScene(char* filename)
{
    // Create the FBX SDK manager
    lSdkManager = FbxManager::Create();

    // Create an IOSettings object.
    FbxIOSettings * ios = FbxIOSettings::Create(lSdkManager, IOSROOT );
    lSdkManager->SetIOSettings(ios);

    // Create the scene
    lFbxScene = FbxScene::Create( lSdkManager, "" );

    // Create an Importer
    FbxImporter* lImporter = FbxImporter::Create( lSdkManager, "" );

    lImporter->Initialize( filename );
    lImporter->Import( lFbxScene );

    lFileName = lImporter->GetFileName().Buffer();

    lImporter->Destroy();

    ProcessScene();
}

void ProcessScene()
{
    ProcessNode( lFbxScene->GetRootNode(), FbxNodeAttribute::eMesh );
}

void ProcessNode(FbxNode* node, int attributeType)
{
    if( !node )
        return;

    FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();

    if( nodeAttribute )
    {
        switch( nodeAttribute->GetAttributeType() )
        {
        case FbxNodeAttribute::eMesh:
            ProcessMesh( node );
        }
    }

    for( int i = 0; i < node->GetChildCount(); i++ )
        ProcessNode( node->GetChild( i ), attributeType );

}

void ProcessMesh( FbxNode* node )
{
    FbxGeometryConverter GeometryConverter( node->GetFbxManager() );
    if( !GeometryConverter.TriangulateInPlace( node ) )
        return;

    FbxMesh* mesh = node->GetMesh();
    if( !mesh )
        return;

    int vertexCount = mesh->GetControlPointsCount();
    if( vertexCount <= 0 )
        return;

    cout << node->GetName() << "\t" << vertexCount << endl;

    FbxVector4* controlPoints = mesh->GetControlPoints();

    for( int i = 0; i < mesh->GetPolygonCount(); ++i )
    {
        meshMatMax = 0;
        FbxLayerElementMaterial* pLayerMaterial = mesh->GetLayer(0)->GetMaterials();
        FbxLayerElementArrayTemplate<int> *tmpArray = &pLayerMaterial->GetIndexArray();
        if( tmpArray->GetAt( i ) > meshMatMax )
            meshMatMax = tmpArray->GetAt( i );
        int texNumVal;
        for( int j = 0; j < 3; ++j )
        {
            int vertexIndex = mesh->GetPolygonVertex( i, j );

            if( vertexIndex < 0 || vertexIndex >= vertexCount )
                continue;

            // Get vertices
            FbxVector4 position = controlPoints[vertexIndex];

            // Get normal
            FbxVector4 normal;
            mesh->GetPolygonVertexNormal( i, j, normal );

            // Get TexCoords
            FbxVector2 texcoord = GetTexCoords( mesh, 0, i, j, vertexIndex );

            // Get Binormals WIP USE AT OWN RISK THERE IS NO ERROR CHECKING HERE SO MUAHAHAHAHA
            FbxGeometryElementBinormal* bin = mesh->GetElementBinormal(0);
            FbxGeometryElementTangent* tan  = mesh->GetElementTangent(0);


            Vertex temp;
            temp.Pos = XMFLOAT3( (float)position[0], (float)position[1], (float)position[2] );
            temp.Normal   = XMFLOAT3( (float)normal[0], (float)normal[1], (float)normal[2] );
            temp.Tex = XMFLOAT2( (float)texcoord[0], (float)(1.0f - texcoord[1]) );
            temp.texNum = tmpArray->GetAt(i) + TexCount;
            texNumVal = temp.texNum;
            //temp.Tangent = temp.Pos;

            //temp.BiNormal;
            //XMVECTOR p = XMVectorSet( temp.Pos.x, temp.Pos.y, temp.Pos.z ,1.0 );
            //XMVECTOR n = XMVectorSet( temp.Normal.x, temp.Normal.y, temp.Normal.z ,1.0 );
            //XMVECTOR cross = XMVector3Cross(p, n);
            //XMStoreFloat3(&temp.BiNormal, cross);
            //D3DXVec3Cross( &temp.BiNormal, &temp.Normal, &temp.Pos );

            //temp.Tangent = XMFLOAT3( tan->GetDirectArray().GetAt(i)[0],
            //							tan->GetDirectArray().GetAt(i)[1],
            //							tan->GetDirectArray().GetAt(i)[2] );
            //
            //temp.BiNormal = XMFLOAT3( bin->GetDirectArray().GetAt(i)[0],
            //							 bin->GetDirectArray().GetAt(i)[1],
            //							 bin->GetDirectArray().GetAt(i)[2] );



            // Add to the Vector
            if( texNumVal+1 > (int)Vertices->size() )
            {
                vector<Vertex> newVec;
                Vertices->push_back(newVec);
            }
            Vertices[0][temp.texNum].push_back( temp );//.push_back( temp );
        }
        if( Vertices[texNumVal].size() % 3 == 0 && Vertices[texNumVal].size() > 0  )
        {
            int i0 = Vertices[texNumVal].size() - 3;
            int i1 = Vertices[texNumVal].size() - 2;
            int i2 = Vertices[texNumVal].size() - 1;

            XMFLOAT3 Q11;
            Q11.x = Vertices[0][texNumVal][i1].Pos.x - Vertices[0][texNumVal][i0].Pos.x;
            Q11.y = Vertices[0][texNumVal][i1].Pos.y - Vertices[0][texNumVal][i0].Pos.y;
            Q11.z = Vertices[0][texNumVal][i1].Pos.z - Vertices[0][texNumVal][i0].Pos.z;
            XMVECTOR Q1 = XMVectorSet( Q11.x, Q11.y, Q11.z, 1.0 );


            XMFLOAT3 Q22;
            Q22.x = Vertices[0][texNumVal][i2].Pos.x - Vertices[0][texNumVal][i0].Pos.x;
            Q22.y = Vertices[0][texNumVal][i2].Pos.y - Vertices[0][texNumVal][i0].Pos.y;
            Q22.z = Vertices[0][texNumVal][i2].Pos.z - Vertices[0][texNumVal][i0].Pos.z;
            XMVECTOR Q2 = XMVectorSet( Q22.x, Q22.y, Q22.z, 1.0 );


            XMFLOAT2 u0 = Vertices[0][texNumVal][i0].Tex;
            XMFLOAT2 u1 = Vertices[0][texNumVal][i1].Tex;
            XMFLOAT2 u2 = Vertices[0][texNumVal][i2].Tex;

            XMFLOAT2 s1;
            s1.x = u1.x - u0.x;
            s1.y = u1.y - u0.y;

            XMFLOAT2 s2;
            s2.x = u2.x - u0.x;
            s2.y = u2.y - u0.y;

            XMVECTOR T = Q1*s2.y  + Q2*-s1.y;
            XMVECTOR B = Q1*-s2.x + Q2*s1.x;

            float val = (float)( 1.f / ( s1.x*s2.y - s2.x*s1.y ) / 2.f );

            T = T * val;
            B = B * val;

            XMStoreFloat3( &Vertices[0][texNumVal][i0].Tangent, T );
            XMStoreFloat3( &Vertices[0][texNumVal][i1].Tangent, T );
            XMStoreFloat3( &Vertices[0][texNumVal][i2].Tangent, T );

            XMStoreFloat3( &Vertices[0][texNumVal][i0].BiNormal, B );
            XMStoreFloat3( &Vertices[0][texNumVal][i1].BiNormal, B );
            XMStoreFloat3( &Vertices[0][texNumVal][i2].BiNormal, B );

        }
    }
    TexCount += meshMatMax+1;
}

FbxVector2 GetTexCoords( FbxMesh* mesh, int layerIndex, int polygonIndex, int polygonVertexIndex, int vertexIndex )
{
    int layerCount = mesh->GetLayerCount();

    if( layerIndex < layerCount )
    {
        FbxLayer* layer = mesh->GetLayer( layerIndex );

        if( layer )
        {
            FbxLayerElementUV* uv = layer->GetUVs( FbxLayerElement::eTextureDiffuse );

            if( uv )
            {
                FbxLayerElement::EMappingMode mappingMode = uv->GetMappingMode();
                FbxLayerElement::EReferenceMode referenceMode = uv->GetReferenceMode();

                const FbxLayerElementArrayTemplate<KFbxVector2>& pUVArray = uv->GetDirectArray();
                const FbxLayerElementArrayTemplate<int>& pUVIndexArray = uv->GetIndexArray();

                switch(mappingMode)
                {
                case FbxLayerElement::eByControlPoint:
                    {
                        int mappingIndex = vertexIndex;
                        switch(referenceMode)
                        {
                        case KFbxLayerElement::eDirect:
                            if( mappingIndex < pUVArray.GetCount() )
                            {
                                return pUVArray.GetAt( mappingIndex );
                            }
                            break;
                        case KFbxLayerElement::eIndexToDirect:
                            if( mappingIndex < pUVIndexArray.GetCount() )
                            {
                                int nIndex = pUVIndexArray.GetAt( mappingIndex );
                                if( nIndex < pUVArray.GetCount() )
                                {
                                    return pUVArray.GetAt( nIndex );
                                }
                            }
                            break;
                        };
                    }
                    break;

                case KFbxLayerElement::eByPolygonVertex:
                    {
                        int mappingIndex = mesh->GetTextureUVIndex( polygonIndex, polygonVertexIndex, KFbxLayerElement::eTextureDiffuse );
                        switch(referenceMode)
                        {
                        case KFbxLayerElement::eDirect:
                        case KFbxLayerElement::eIndexToDirect: //I have no idea why the index array is not used in this case.
                            if( mappingIndex < pUVArray.GetCount() )
                            {
                                return pUVArray.GetAt( mappingIndex );
                            }
                            break;
                        };
                    }
                    break;
                }
            }
        }
        return FbxVector2();
    }
    return FbxVector2();
}