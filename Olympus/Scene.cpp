#include "Scene.h"

Scene::Scene(vector<Renderable*>* renderables, ID3D11Device *dev, ID3D11DeviceContext *devcon, Apex* apex, GeometryGenerator *geoGen,
			Renderable *skyBox,	ScreenQuad *screenQuad, ID3D11DepthStencilView *zbuff, D3D11_VIEWPORT *screenViewport) :
			mRenderables(renderables), mDev(dev), mDevcon(devcon), mApex(apex), mGeoGen(geoGen), mSkyBox(skyBox), mScreen(screenQuad),
			mZbuffer(zbuff), mScreenViewport(screenViewport)
{
	std::ifstream fin("Scene/scene.txt");
    if(!fin)
    {
        MessageBox(0, "Scene/scene.txt not found.", 0, 0);
        return;
    }

	vector<string> objectFilenames;
	vector<string> sphereFilenames;
	vector<string> physxFilenames;
	std::vector<string> elems;
	string lines, olines;
	string item;
	while(getline(fin, lines))
	{
		if(!lines.length()) continue; //skip empty
		if (lines[0] == '#') //skip comments
		{
			continue;
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

			Object* bowlingPin = new Object();
			char *modelname = "Media/Models/bowling_pin_lowres.fbx";
			bowlingPin->objLoad(modelname, &texts, &norms, mDev, mDevcon, mApex );
		
			PlacePins( XMFLOAT3(::atof(elems[1].c_str()), ::atof(elems[2].c_str()), ::atof(elems[3].c_str()) ), 15, 2.0f, bowlingPin);
			
			mRenderables->push_back(bowlingPin);
			bowlingSets.push_back(bowlingPin);
			elems.clear();
		}
	}
	fin.close();

    for(int i = 0; i < objectFilenames.size(); i++)
    {
		LoadFBX(objectFilenames[i]);
    }
	for(int i = 0; i < sphereFilenames.size(); i++)
    {
		LoadSpheres(sphereFilenames[i]);
    }
	for(int i = 0; i < physxFilenames.size(); i++)
    {
		LoadPhysX(physxFilenames[i]);
    }
}

Scene::~Scene()
{

}

void Scene::LoadFBX(string filename)
{
	string model;
	vector<string> textures;
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
			while(std::getline(ss, item, ' ')) {
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
			while(std::getline(ss, item, ' ')) {
				elems.push_back(item);
			}
			textures.push_back(elems[1]);
			elems.clear();
		}
		else if (olines[0] == 'n') 
		{
			string* sline;
			sline = &olines;
			stringstream ss(*sline);
			while(std::getline(ss, item, ' ')) {
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
			while(std::getline(ss, item, ' ')) {
				elems.push_back(item);
			}
			ObjectInfo object;
			object.x = ::atof(elems[1].c_str());
			object.y = ::atof(elems[2].c_str());
			object.z = ::atof(elems[3].c_str());
			object.sx = ::atof(elems[4].c_str());
			object.sy = ::atof(elems[5].c_str());
			object.sz = ::atof(elems[6].c_str());
			object.rx = ::atof(elems[7].c_str());
			object.ry = ::atof(elems[8].c_str());
			object.rz = ::atof(elems[9].c_str());

			objInfos.push_back(object);
            instances++;
			elems.clear();
		}
        else if (olines[0] == 'a') 
		{
			string* sline;
			sline = &olines;
			stringstream ss(*sline);
			while(std::getline(ss, item, ' ')) {
				elems.push_back(item);
			}

            Material material;
            material.Ambient.x = ::atof(elems[1].c_str());
			material.Ambient.y = ::atof(elems[2].c_str());
			material.Ambient.z = ::atof(elems[3].c_str());
			material.Ambient.w = ::atof(elems[4].c_str());

            material.Diffuse.x = ::atof(elems[5].c_str());
			material.Diffuse.y = ::atof(elems[6].c_str());
			material.Diffuse.z = ::atof(elems[7].c_str());
			material.Diffuse.w = ::atof(elems[8].c_str());

            material.Specular.x = ::atof(elems[9].c_str());
			material.Specular.y = ::atof(elems[10].c_str());
			material.Specular.z = ::atof(elems[11].c_str());
			material.Specular.w = ::atof(elems[12].c_str());

			material.Reflect.x = ::atof(elems[13].c_str());
			material.Reflect.y = ::atof(elems[14].c_str());
			material.Reflect.z = ::atof(elems[15].c_str());
			material.Reflect.w = ::atof(elems[16].c_str());

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
	for(int k = 0; k < textures.size(); k++)
	{
		texts.push_back(textures[k].c_str());
	}
	for(int k = 0; k < normals.size(); k++)
	{
		norms.push_back(normals[k].c_str());
	}
	Object* object = new Object();
		
	char *modelname = new char[model.length() + 1];
	strcpy(modelname, model.c_str());

	object->objLoad(modelname, &texts, &norms, mDev, mDevcon, mApex );
		
	for(int k = 0; k < objInfos.size(); k++)
	{
		object->AddInstance(objInfos[k]);
	}

	mRenderables->push_back(object);

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
			while(std::getline(ss, item, ' ')) {
				elems.push_back(item);
			}

			Sphere* sphere = new Sphere(mDevcon, mDev, mGeoGen, mApex, ::atoi(elems[1].c_str()), ::atoi(elems[2].c_str()), ::atoi(elems[3].c_str()) );
			sphere->MoveTo( ::atof(elems[4].c_str()), ::atof(elems[5].c_str()), ::atof(elems[6].c_str()) );
			mRenderables->push_back(sphere);
	
			elems.clear();
		}
		else if (olines[0] == 'r') 
		{
			string* sline;
			sline = &olines;
			stringstream ss(*sline);
			while(std::getline(ss, item, ' ')) {
				elems.push_back(item);
			}

			Sphere* sphere = new Sphere(mDevcon, mDev, mGeoGen, mApex, ::atoi(elems[1].c_str()), ::atoi(elems[2].c_str()), ::atoi(elems[3].c_str()) );
			sphere->MoveTo( ::atof(elems[4].c_str()), ::atof(elems[5].c_str()), ::atof(elems[6].c_str()) );
			sphere->SetupReflective(mRenderables, mSkyBox, mScreen, mZbuffer, mScreenViewport);
			mRenderables->push_back(sphere);
			
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
			while(std::getline(ss, item, ' ')) {
				elems.push_back(item);
			}

			ApexCloth* mCloth = mApex->CreateCloth(gRenderer, elems[1].c_str() );//"curtainew");//"ctdm_Cape_400");//"bannercloth");//
			mRenderables->push_back(mCloth);
			cloths.push_back(mCloth);
			elems.clear();
		}
		else if (olines[0] == 'e') 
		{
			string* sline;
			sline = &olines;
			stringstream ss(*sline);
			while(std::getline(ss, item, ' ')) {
				elems.push_back(item);
			}

			ApexParticles* emitter = mApex->CreateEmitter(gRenderer, elems[1].c_str());//"SmokeEmitter");
			emitter->SetPosition(::atof(elems[2].c_str()), ::atof(elems[3].c_str()), ::atof(elems[4].c_str()));
			mRenderables->push_back(emitter);

			elems.clear();
		}
		else if (olines[0] == 'p') 
		{
			string* sline;
			sline = &olines;
			stringstream ss(*sline);
			while(std::getline(ss, item, ' ')) {
				elems.push_back(item);
			}
			// Create plane 
			elems.clear();
		}
	}
	oin.close();

	projectile = new Projectile(mDev, mDevcon, mApex);
	mRenderables->push_back(projectile);
}

void Scene::Fire(Camera *mCam, float speed)
{
	if( projectile && (cloths.size() > 0) )
	{
		projectile->Fire(mCam, speed, cloths[0]);
	}
}

void Scene::PlacePins(XMFLOAT3 location, int numlevels, float dist, Object* pinModel)
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
	object.sx = object.sy = object.sz = 7.0f;
	object.materials.push_back(material);

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

void Scene::Update()
{
	for(int i = 0; i < bowlingSets.size(); i++)
	{
		for(int j = 0; j < bowlingSets[i]->mWorldMats.size(); j++)
		{
			mApex->getRigidDynamicPosition(j, &bowlingSets[i]->mWorldMats[j]);
		}
	}
}