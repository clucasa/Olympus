#include "Scene.h"

Scene::Scene( ID3D11Device *dev, ID3D11DeviceContext *devcon, Apex* apex, GeometryGenerator *geoGen,
            Renderable *skyBox,	ScreenQuad *screenQuad, ID3D11DepthStencilView *zbuff, D3D11_VIEWPORT *screenViewport, String sceneName) :
            mDev(dev), mDevcon(devcon), mApex(apex), mGeoGen(geoGen), mSkyBox(skyBox), mScreen(screenQuad),
            mZbuffer(zbuff), mScreenViewport(screenViewport)
{
    //Used for resetting the pins
    mPinStartPosition = XMFLOAT3(0,0,0);
    mDist			  = 0;
    mNumLevels		  = -1;

    mGravity = 0;

	mMaxProjectile = 0;
	mSpeedScale = 0;

    std::ifstream fin(sceneName);
    if(!fin)
    {
        MessageBox(0, "Scene/scene.txt not found.", 0, 0);
        return;
    }

    vector<string> objectFilenames;
    vector<string> sphereFilenames;
    vector<string> physxFilenames;
    vector<string> settingsFilenames;
    std::vector<string> elems;
    string lines, olines;
    string item;
	int jengaCheck = 0;
    while(getline(fin, lines))
    {
        if(!lines.length()) continue; //skip empty
        if (lines[0] == '#') //skip comments
        {
            continue;
        }
		else if (lines[0] == 't') 
        {
            string* sline;
            sline = &lines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) {
                elems.push_back(item);
            }
			LoadSettings(elems[1]);
            elems.clear();
        }
        else if (lines[0] == 'o') 
        {
            string* sline;
            sline = &lines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) {
                elems.push_back(item);
            }
            objectFilenames.push_back(elems[1]);
            elems.clear();
        }
        else if (lines[0] == 's') 
        {
            string* sline;
            sline = &lines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) {
                elems.push_back(item);
            }
            sphereFilenames.push_back(elems[1]);
            elems.clear();
		}
        else if (lines[0] == 'p') 
        {
            string* sline;
            sline = &lines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) {
                elems.push_back(item);
            }
            physxFilenames.push_back(elems[1]);
            elems.clear();
        }
        else if (lines[0] == 'b') 
        {
            string* sline;
            sline = &lines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) {
                elems.push_back(item);
            }
            //physxFilenames.push_back(elems[1]);
            vector<LPCSTR> texts;
            vector<LPCSTR> norms;
            texts.push_back("Media/Textures/bowling_pin.png");
            norms.push_back("Media/Textures/BlankNormalMap.png");
			vector<float> texscale;
			texscale.push_back(1.0f);
            Object* bowlingPin = new Object();
            char *modelname = "Media/Models/bowling_pin_lowres.fbx";
            bowlingPin->objLoad(modelname, &texts, &norms, texscale, mDev, mDevcon, mApex );
        
            PlacePins( XMFLOAT3((float)::atof(elems[1].c_str()), (float)::atof(elems[2].c_str()), (float)::atof(elems[3].c_str()) ),
                (int)::atoi(elems[4].c_str()), (float)::atof(elems[5].c_str()), (float)::atof(elems[6].c_str()),  bowlingPin);
            
            mRenderables.push_back(bowlingPin);
            bowlingSets.push_back(bowlingPin);
            elems.clear();
        }
        else if (lines[0] == 'j') 
        {
            string* sline;
            sline = &lines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) {
                elems.push_back(item);
            }
            
            PlaceJenga( XMFLOAT3((float)::atof(elems[1].c_str()), (float)::atof(elems[2].c_str()), (float)::atof(elems[3].c_str()) ),
                (int)::atoi(elems[4].c_str()), (float)::atof(elems[5].c_str()), (float)::atof(elems[6].c_str()));
            
			jengaCheck = 1;
			
            /*mRenderables.push_back(bowlingPin);
            bowlingSets.push_back(bowlingPin);*/
            elems.clear();
        }
    }
    fin.close();
	if(jengaCheck == 1)
	{
		mRenderables.push_back(jengaBlock1);
		mRenderables.push_back(jengaBlock2);
		jengaCheck = 0;
	}	

	/*for(int i = 0; i < (int)settingsFilenames.size(); i++)
    {
        LoadSettings(settingsFilenames[i]);
    }*/
    for(int i = 0; i < (int)objectFilenames.size(); i++)
    {
        LoadFBX(objectFilenames[i]);
    }
    for(int i = 0; i < (int)sphereFilenames.size(); i++)
    {
        LoadSpheres(sphereFilenames[i]);
    }
    for(int i = 0; i < (int)physxFilenames.size(); i++)
    {
        LoadPhysX(physxFilenames[i]);
    }
}

Scene::~Scene()
{

}

void Scene::LoadSettings(string filename)
{
    std::vector<string> elems;
    string olines;
    string item;

    std::ifstream oin(filename);
    if(!oin)
    {
        MessageBox(0, "settings.txt not found.", 0, 0);
        return;
    }

    while(getline(oin, olines))
    {
        if(!olines.length()) continue; //skip empty
        if (olines[0] == '#') //skip comments
        {
            continue;
        }
        else if (olines[0] == 'g') 
        {
            stringstream ss(olines);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }
			mGravity = (float)::atof(elems[1].c_str());

            elems.clear();
        }
        else if (olines[0] == 'p') 
        {
            stringstream ss(olines);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }
			mMaxProjectile = ::atoi(elems[1].c_str());
            elems.clear();
        }
        else if (olines[0] == 's') 
        {
            stringstream ss(olines);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }
			mSpeedScale = (float)::atof(elems[1].c_str());
            elems.clear();
        }
    }
    oin.close();

	mApex->CreateScene(mGravity);
	cController = new CharacterController(mApex);
}

void Scene::LoadFBX(string filename)
{
    string model;
    vector<string> textures;
	vector<float> textureScale;
    vector<string> normals;
    vector<ObjectInfo> objInfos;
    std::vector<string> elems;
    string  olines;
    string item;

    std::ifstream oin(filename);
    if(!oin)
    {
        MessageBox(0, "object.txt not found.", 0, 0);
        return;
    }
    int instances = 0;
    while(getline(oin, olines))
    {
        if(!olines.length()) continue; //skip empty
        if (olines[0] == '#') //skip comments
        {
            continue;
        }
        else if (olines[0] == 'm') 
        {
            stringstream ss(olines);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }
            model = elems[1];
            elems.clear();
        }
        else if (olines[0] == 't') 
        {
            string* sline;
            sline = &olines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }
            textures.push_back(elems[1]);
			if(elems.size() > 2)
				textureScale.push_back((float)::atof(elems[2].c_str()));
			else
				textureScale.push_back(1.0f);
            elems.clear();
        }
        else if (olines[0] == 'n') 
        {
            string* sline;
            sline = &olines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }
            normals.push_back(elems[1]);
            elems.clear();
        }
        else if (olines[0] == 'i') 
        {
            string* sline;
            sline = &olines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }
            ObjectInfo object;
            object.x = (float)::atof(elems[1].c_str());
            object.y = (float)::atof(elems[2].c_str());
            object.z = (float)::atof(elems[3].c_str());
            object.sx = (float)::atof(elems[4].c_str());
            object.sy = (float)::atof(elems[5].c_str());
            object.sz = (float)::atof(elems[6].c_str());
            object.rx = (float)::atof(elems[7].c_str());
            object.ry = (float)::atof(elems[8].c_str());
            object.rz = (float)::atof(elems[9].c_str());

            objInfos.push_back(object);
            instances++;
            elems.clear();
        }
        else if (olines[0] == 'a') 
        {
            string* sline;
            sline = &olines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }

            Material material;
            material.Ambient.x = (float)::atof(elems[1].c_str());
            material.Ambient.y = (float)::atof(elems[2].c_str());
            material.Ambient.z = (float)::atof(elems[3].c_str());
            material.Ambient.w = (float)::atof(elems[4].c_str());

            material.Diffuse.x = (float)::atof(elems[5].c_str());
            material.Diffuse.y = (float)::atof(elems[6].c_str());
            material.Diffuse.z = (float)::atof(elems[7].c_str());
            material.Diffuse.w = (float)::atof(elems[8].c_str());

            material.Specular.x = (float)::atof(elems[9].c_str());
            material.Specular.y = (float)::atof(elems[10].c_str());
            material.Specular.z = (float)::atof(elems[11].c_str());
            material.Specular.w = (float)::atof(elems[12].c_str());

            material.Reflect.x = (float)::atof(elems[13].c_str());
            material.Reflect.y = (float)::atof(elems[14].c_str());
            material.Reflect.z = (float)::atof(elems[15].c_str());
            material.Reflect.w = (float)::atof(elems[16].c_str());

            material.alphaKillOn = ::atoi(elems[17].c_str());
            material.backCullOn = 0;
            material.dynamicOn = 0;

            objInfos[instances-1].materials.push_back(material);
            elems.clear();
        }
    }
    oin.close();

    vector<LPCSTR> texts;
    vector<LPCSTR> norms;
    for(int k = 0; k < (int)textures.size(); k++)
    {
        texts.push_back(textures[k].c_str());
    }
    for(int k = 0; k < (int)normals.size(); k++)
    {
        norms.push_back(normals[k].c_str());
    }
    Object* object = new Object();
        
    char *modelname = new char[model.length() + 1];
    strcpy_s(modelname, model.length()+1, model.c_str());

    object->objLoad(modelname, &texts, &norms, textureScale, mDev, mDevcon, mApex );
        
    for(int k = 0; k < (int)objInfos.size(); k++)
    {
        object->AddInstance(objInfos[k]);
    }

    mRenderables.push_back(object);

    textures.clear();
    normals.clear();
    objInfos.clear();
    model = "";
}

void Scene::LoadSpheres(string filename)
{
    std::vector<string> elems;
    string olines;
    string item;

    std::ifstream oin(filename);
    if(!oin)
    {
        MessageBox(0, "spheres.txt not found.", 0, 0);
        return;
    }
    int instances = 0;
    while(getline(oin, olines))
    {
        if(!olines.length()) continue; //skip empty
        if (olines[0] == '#') //skip comments
        {
            continue;
        }
        else if (olines[0] == 's') 
        {
            stringstream ss(olines);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }

            Sphere* sphere = new Sphere(mDevcon, mDev, mGeoGen, mApex, ::atoi(elems[1].c_str()), ::atoi(elems[2].c_str()), ::atoi(elems[3].c_str()) );
            sphere->MoveTo( (float)::atof(elems[4].c_str()), (float)::atof(elems[5].c_str()), (float)::atof(elems[6].c_str()) );
            mRenderables.push_back(sphere);
    
            elems.clear();
        }
        else if (olines[0] == 'r') 
        {
            string* sline;
            sline = &olines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }

            Sphere* sphere = new Sphere(mDevcon, mDev, mGeoGen, mApex, ::atoi(elems[1].c_str()), ::atoi(elems[2].c_str()), ::atoi(elems[3].c_str()) );
            sphere->MoveTo( (float)::atof(elems[4].c_str()), (float)::atof(elems[5].c_str()), (float)::atof(elems[6].c_str()) );
            sphere->SetupReflective(&mRenderables, mSkyBox, mScreen, mZbuffer, mScreenViewport);
            mRenderables.push_back(sphere);
            reflectiveSpheres.push_back(sphere);
            elems.clear();
        }
    }
    oin.close();

}

void Scene::LoadPhysX(string filename)
{
    std::vector<string> elems;
    string  olines;
    string item;

    physx::apex::NxUserRenderer* gRenderer = new ZeusRenderer();

    std::ifstream oin(filename);
    if(!oin)
    {
        MessageBox(0, "object.txt not found.", 0, 0);
        return;
    }
    int instances = 0;
    while(getline(oin, olines))
    {
        if(!olines.length()) continue; //skip empty
        if (olines[0] == '#') //skip comments
        {
            continue;
        }
        else if (olines[0] == 'c') 
        {
            stringstream ss(olines);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }

            ApexCloth* mCloth = mApex->CreateCloth(gRenderer, elems[1].c_str(), elems[8].c_str(), (float)::atof(elems[9].c_str()) );//"curtainew");//"ctdm_Cape_400");//"bannercloth");//
            mCloth->SetPosition((float)::atof(elems[2].c_str()), (float)::atof(elems[3].c_str()), (float)::atof(elems[4].c_str()),
                (float)::atof(elems[5].c_str()), (float)::atof(elems[6].c_str()), (float)::atof(elems[7].c_str()) );
            mRenderables.push_back(mCloth);
            cloths.push_back(mCloth);

            elems.clear();
        }
        else if (olines[0] == 'e') 
        {
            string* sline;
            sline = &olines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }

            ApexParticles* emitter = mApex->CreateEmitter( gRenderer, elems[1].c_str(), elems[5].c_str() );//"SmokeEmitter");
            emitter->SetPosition((float)::atof(elems[2].c_str()), (float)::atof(elems[3].c_str()), (float)::atof(elems[4].c_str()));
            mBlendRenderables.push_back(emitter);
            particles.push_back(emitter);
            elems.clear();
        }
        else if (olines[0] == 'p') 
        {
            string* sline;
            sline = &olines;
            stringstream ss(*sline);
            while(std::getline(ss, item, ' ')) 
			{
                elems.push_back(item);
            }

            // Create plane 
            mApex->CreatePlane((float)::atof(elems[1].c_str()), (float)::atof(elems[2].c_str()), (float)::atof(elems[3].c_str()), (float)::atof(elems[4].c_str()));

            elems.clear();
        }
    }
    oin.close();

    projectile = new Projectile(mDev, mDevcon, mApex, mMaxProjectile);
    mRenderables.push_back(projectile);
}

void Scene::Fire(Camera *mCam, float speed)
{
    if( projectile )
    {
        projectile->Fire(mCam, speed, cloths);
    }
}

void Scene::ClearProjectiles()
{
	if( projectile )
    {
        projectile->Clear();
    }
}

void Scene::PlacePins(XMFLOAT3 location, int numlevels, float dist, float scale, Object* pinModel)
{	
    Material material;
    material.Ambient.x = material.Ambient.y = material.Ambient.z = 0.2f;
    material.Ambient.w = material.Diffuse.w = 1.0f;

    material.Diffuse.x = material.Diffuse.y = material.Diffuse.z = 0.3f;
    material.Specular.x = material.Specular.y = material.Specular.z = 0.9f;

    material.Specular.w = 100.0f;

    material.Reflect.x = material.Reflect.y = material.Reflect.z = material.Reflect.w = 0.0f;

    material.alphaKillOn = 0;
    material.backCullOn = 0;
    material.dynamicOn = 1;

    ObjectInfo object;
    object.x = object.y = object.z = object.rx = object.ry = object.rz = 0.0f;
    object.sx = object.sy = object.sz = scale;
    object.materials.push_back(material);

    //Used for resetting the pins
    mPinStartPosition = location;
    mDist			  = dist;
    mNumLevels		  = numlevels;

    XMFLOAT3 startLocation = location;
    XMFLOAT3 tempLocation = location;
       
    for(int i = 0; i <= numlevels; i++)
    {
        for(int j = 0; j<=i; j++)
        {
            object.x = tempLocation.x;
            object.y = tempLocation.y;
            object.z = tempLocation.z;
            pinModel->AddInstance(object);
            tempLocation.x -= dist;
        }
        
        startLocation.x += dist/2.f;
        startLocation.z += sqrt((dist)*(dist)*(1.f - (1.f/4.f)));

        tempLocation = startLocation;              
    }

    if(numlevels == 0)
    {
        object.x = location.x;
        object.y = location.y;
        object.z = location.z;
        pinModel->AddInstance(object);
    }
}

void Scene::ResetPins()
{	
    if(mNumLevels < 0)
        return;

    XMFLOAT3 startLocation = mPinStartPosition;
    XMFLOAT3 tempLocation = mPinStartPosition;
    float dist = mDist;
    int   numlevels = mNumLevels;

    PxVec3 pos;
    
    int currentPin = 0;

    for(int i = 0; i <= numlevels; i++)
    {
        for(int j = 0; j<=i; j++)
        {
            pos = PxVec3(tempLocation.x, tempLocation.y, tempLocation.z);

            PxTransform transform(pos, PxQuat::createIdentity());


            PxRigidDynamic* boxDynamic = static_cast<PxRigidDynamic*>(mApex->dynamicActors[currentPin++]);
            boxDynamic->setLinearVelocity(PxVec3(0,0,0));
            boxDynamic->setGlobalPose(transform);
            boxDynamic->setAngularVelocity(PxVec3(0,0,0));
            

            
            tempLocation.x -= dist;
        }
        
        startLocation.x += dist/2.f;
        startLocation.z += sqrt((dist)*(dist)*(1.f - (1.f/4.f)));

        tempLocation = startLocation;              
    }

    if(numlevels == 0)
    {
        pos = PxVec3(tempLocation.x, tempLocation.y, tempLocation.z);
        PxTransform transform(pos, PxQuat::createIdentity());

        mApex->dynamicActors[0]->setGlobalPose(transform);
        PxRigidDynamic* boxDynamic = static_cast<PxRigidDynamic*>(mApex->dynamicActors[0]);
        boxDynamic->setLinearVelocity(PxVec3(0,0,0));
        boxDynamic->setGlobalPose(transform);
        boxDynamic->setAngularVelocity(PxVec3(0,0,0));

    }
}


float oldLength = -1;
int blocks = 0;

void Scene::PlaceJenga(XMFLOAT3 location, int numlevels, float dist, float length)
{
    //Used for resetting the pins
    mJengaStartPosition.push_back(location);
	mJengaNumLevels.push_back(numlevels);
	mJengaLength.push_back(length);

    //mJengaDist is "fudge" factor, incase we need to add a bit less due to objects appearing inside each other
    mJengaDist			  = dist;
    
    XMFLOAT3 startLocation = location;

    float width	 = (length / 3.0f);
    float height = (length / 5.0f);

	bool different = false;

	if(mJengaStartPosition.size() == 1 || length != oldLength)
	{
		if(mJengaStartPosition.size() > 1)
		{
			mRenderables.push_back(jengaBlock1);
			mRenderables.push_back(jengaBlock2);
		}

		different = true;
		jengaBlock1 = new Box(mDevcon, mDev, mApex, length, width, height);
		jengaBlock2 = new Box(mDevcon, mDev, mApex, width, length, height);

	}

	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(mDev, "Media/Textures/Wood.png", 0, 0, &jengaBlock1->mTexture, 0 );
	//hr = D3DX11CreateShaderResourceViewFromFile(mDev, "Media/Textures/bricks_nmap.dds", 0, 0, &jengaBlock1->mNmap, 0 );
	
	hr = D3DX11CreateShaderResourceViewFromFile(mDev, "Media/Textures/Wood.png", 0, 0, &jengaBlock2->mTexture, 0 );
	//hr = D3DX11CreateShaderResourceViewFromFile(mDev, "Media/Textures/bricks_nmap.dds", 0, 0, &jengaBlock2->mNmap, 0 );


	oldLength = length;

    XMFLOAT3 midBlock, rightBlock, leftBlock;

    for(int i = 0; i < numlevels; i++)
    {
        if((i % 2) == 1)
        {
            midBlock	= startLocation;
            midBlock.y  += (float)i*height + dist;

            rightBlock = startLocation;
            rightBlock.z += width + dist;
            rightBlock.y += (float)i*height + dist;

            leftBlock = startLocation;
            leftBlock.z -= width + dist;
            leftBlock.y += (float)i*height + dist;

            PlaceBlock(midBlock, length, width, height, 0);
            PlaceBlock(rightBlock, length, width, height, 0);
            PlaceBlock(leftBlock, length, width, height, 0);

            jengaBlock1->AddInstance(midBlock.x, midBlock.y, midBlock.z);
            jengaBlock1->AddInstance(rightBlock.x, rightBlock.y, rightBlock.z);
            jengaBlock1->AddInstance(leftBlock.x, leftBlock.y, leftBlock.z);
        }
        else
        {
            midBlock	= startLocation;
            midBlock.y  += (float)i*height + dist;

            rightBlock = startLocation;
            rightBlock.x += width + dist;
            rightBlock.y += (float)i*height + dist;

            leftBlock = startLocation;
            leftBlock.x -= width + dist;
            leftBlock.y += (float)i*height + dist;

            PlaceBlock(midBlock, width, length, height, 1);
            PlaceBlock(rightBlock, width, length, height, 1);
            PlaceBlock(leftBlock, width, length, height, 1);

            jengaBlock2->AddInstance(midBlock.x, midBlock.y, midBlock.z);
            jengaBlock2->AddInstance(rightBlock.x, rightBlock.y, rightBlock.z);
            jengaBlock2->AddInstance(leftBlock.x, leftBlock.y, leftBlock.z);
        }
    }
    
	if(mJengaStartPosition.size() == 1 || different)
	{
		
		JengaBlocks.push_back(jengaBlock2);
		JengaBlocks.push_back(jengaBlock1);
		blocks += 2;
	}
	else
	{
		JengaBlocks[blocks-2] = jengaBlock2;
		JengaBlocks[blocks-1] = jengaBlock1;
	}
   
}

void Scene::PlaceBlock(XMFLOAT3 location, float length, float width, float height, int side)
{
    PxMaterial*	blockMaterial = mApex->getPhysics()->createMaterial(0.5f, 0.5f, 0.3f);    //static friction, dynamic friction, restitution
    if(!blockMaterial)
        return;

    PxVec3 pos = PxVec3(location.x,location.y+(height/2.f),location.z);
    PxReal density = 0.1f;
        
    PxTransform transform(pos, PxQuat::createIdentity());
    PxVec3 dimensions(length/2.f, height/2.f, width/2.f);
    PxBoxGeometry geometry(dimensions);
    PxRigidDynamic* boxActor = PxCreateDynamic(*mApex->getPhysics(), transform, geometry, *blockMaterial, density);
    if (!boxActor)
        return;

    boxActor->setLinearVelocity(PxVec3(0,0,0));
    boxActor->setAngularDamping((PxReal)0.95f);
    PxRigidBodyExt::updateMassAndInertia(*boxActor, density);

    mApex->getScene()->addActor(*boxActor);
	if(side == 0)
		blocks1.push_back(boxActor);
	else
		blocks2.push_back(boxActor);
}

void Scene::ResetJenga()
{
    
	PxRigidDynamic* blockDynamic = 0;

	float width;
	float height;
    

    PxVec3 midBlock, rightBlock, leftBlock;
	int block1counter = 0;
	int block2counter = 0;

	for(int j = 0 ; j < mJengaStartPosition.size(); j++)
	{
		PxVec3 startLocation = PxVec3(mJengaStartPosition[j].x, mJengaStartPosition[j].y, mJengaStartPosition[j].z);
		width	 = (mJengaLength[j] / 3.f);
		height = (mJengaLength[j] / 5.f);

		for(int i = 0; i < mJengaNumLevels[j]; i++)
		{
			if((i % 2) == 1)
			{
				midBlock	= startLocation;
				midBlock.y  += (float)i*height + mDist+(height/2.f);

				blockDynamic = static_cast<PxRigidDynamic*>(blocks1[block1counter++]);
				blockDynamic->setLinearVelocity(PxVec3(0,0,0));
				blockDynamic->setGlobalPose(PxTransform(midBlock));
				blockDynamic->setAngularVelocity(PxVec3(0,0,0));

				rightBlock = startLocation;
				rightBlock.z += width + mDist;
				rightBlock.y += (float)i*height + mDist+(height/2.f);

				blockDynamic = static_cast<PxRigidDynamic*>(blocks1[block1counter++]);
				blockDynamic->setLinearVelocity(PxVec3(0,0,0));
				blockDynamic->setGlobalPose(PxTransform(rightBlock));
				blockDynamic->setAngularVelocity(PxVec3(0,0,0));

				leftBlock = startLocation;
				leftBlock.z -= width + mDist;
				leftBlock.y += (float)i*height + mDist+(height/2.f);

				blockDynamic = static_cast<PxRigidDynamic*>(blocks1[block1counter++]);
				blockDynamic->setLinearVelocity(PxVec3(0,0,0));
				blockDynamic->setGlobalPose(PxTransform(leftBlock));
				blockDynamic->setAngularVelocity(PxVec3(0,0,0));
			}
			else
			{
				midBlock	= startLocation;
				midBlock.y  += (float)i*height + mDist+(height/2.f);

				blockDynamic = static_cast<PxRigidDynamic*>(blocks2[block2counter++]);
				blockDynamic->setLinearVelocity(PxVec3(0,0,0));
				blockDynamic->setGlobalPose(PxTransform(midBlock));
				blockDynamic->setAngularVelocity(PxVec3(0,0,0));

				rightBlock = startLocation;
				rightBlock.x += width + mDist;
				rightBlock.y += (float)i*height + mDist+(height/2.f);

				blockDynamic = static_cast<PxRigidDynamic*>(blocks2[block2counter++]);
				blockDynamic->setLinearVelocity(PxVec3(0,0,0));
				blockDynamic->setGlobalPose(PxTransform(rightBlock));
				blockDynamic->setAngularVelocity(PxVec3(0,0,0));

				leftBlock = startLocation;
				leftBlock.x -= width + mDist;
				leftBlock.y += (float)i*height + mDist+(height/2.f);
	
				blockDynamic = static_cast<PxRigidDynamic*>(blocks2[block2counter++]);
				blockDynamic->setLinearVelocity(PxVec3(0,0,0));
				blockDynamic->setGlobalPose(PxTransform(leftBlock));
				blockDynamic->setAngularVelocity(PxVec3(0,0,0));
			}
		}		
	}

}


void Scene::Update()
{
    XMMATRIX scale;
    for(int i = 0; i < (int)bowlingSets.size(); i++)
    {
        for(int j = 0; j < (int)bowlingSets[i]->mWorldMats.size(); j++)
        {
            mApex->getRigidDynamicPosition(j, &bowlingSets[i]->mWorldMats[j]);
            
            scale = XMMatrixScaling(bowlingSets[i]->mScale,bowlingSets[i]->mScale,bowlingSets[i]->mScale);
            XMStoreFloat4x4(&bowlingSets[i]->mWorldMats[j], XMMatrixMultiply(scale,  XMLoadFloat4x4(&bowlingSets[i]->mWorldMats[j]) ));
        }
    }
	
	int count1 = 0;
	int count2 = 0;

	for(int i = 0; i < (int)JengaBlocks.size(); i++)
	{
		for(int j = 0; j < (int)JengaBlocks[i]->mWorldMats.size(); j++)
		{
			PxU32 nShapes = 0;
			PxRigidActor* block;
			if( i % 2 == 0 )
			{
				if(blocks1[count1])
					block = blocks1[count1++];
				else	
					return;
			}
			else
			{
				if(blocks2[count2])
					block = blocks2[count2++];
				else	
					return;
			}

			nShapes = block->getNbShapes();
			

			PxShape** shapes = new PxShape*[nShapes];
 
			block->getShapes(shapes, nShapes);     
			PxTransform pt = PxShapeExt::getGlobalPose(*shapes[0]);

			delete [] shapes;

			XMMATRIX world = XMLoadFloat4x4(&JengaBlocks[i]->mWorldMats[j]) ;
			mApex->PxtoXMMatrix(pt, &world);

			XMStoreFloat4x4(&JengaBlocks[i]->mWorldMats[j], world);          
		}
	}

    for(int i = 0; i < (int)mRenderables.size() ; i++)
    {
        mRenderables[i]->Update();
    }
    for(int i = 0; i < (int)mBlendRenderables.size() ; i++)
    {
        mBlendRenderables[i]->Update();
    }
}


void Scene::UpdateZbuffers(ID3D11DepthStencilView *zbuffer)
{
    mZbuffer = zbuffer;
    for(int i = 0; i < (int)reflectiveSpheres.size(); i++)
    {
        reflectiveSpheres[i]->mZbuffer = zbuffer;
    }
}

void Scene::UpdateReflective(Camera *cam)
{
    XMVECTOR camPos = cam->GetPositionXM();

    for(int i = 0; i < (int)reflectiveSpheres.size(); i++)
    {
        XMVECTOR spherePos = XMLoadFloat3(&XMFLOAT3(reflectiveSpheres[i]->mX,reflectiveSpheres[i]->mY,reflectiveSpheres[i]->mZ));
        XMVECTOR vectorSub = XMVectorSubtract(camPos,spherePos);
        XMVECTOR length = XMVector3Length(vectorSub);

        float distance = 0.0f;
        XMStoreFloat(&distance,length);
        if(abs(distance) > 60.0f)
        {
            reflectiveSpheres[i]->IsItReflective(false);
        }
        else if(reflectiveSpheres[i]->reflective == false)
        {
            reflectiveSpheres[i]->IsItReflective(true);
        }
    }
}

void Scene::ToggleParticles(bool on)
{
    for(int i = 0; i < (int)particles.size(); i++)
    {
        particles[i]->SetEmit(on);
    }
}
