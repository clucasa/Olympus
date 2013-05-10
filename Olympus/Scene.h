#pragma once

#ifndef SCENE_H
#define SCENE_H

#include "Object.h"
#include "Sphere.h"
#include <vector>
#include "Projectile.h"

class Apex;
//class Camera;

using namespace std;

class Scene
{
public:
	Scene(	vector<Renderable*>* renderables,ID3D11Device *dev, ID3D11DeviceContext *devcon, Apex* apex, GeometryGenerator *geoGen,
			Renderable *skyBox,	ScreenQuad *screenQuad, ID3D11DepthStencilView *zbuff, D3D11_VIEWPORT *screenViewport);

	~Scene();

	void Fire(Camera *mCam, float speed);

	void LoadFBX(string filename);
	void LoadPhysX(string filename);
	void LoadSpheres(string filename);
	void LoadSettings(string filename);

	void PlacePins(XMFLOAT3 location, int numlevels, float dist, Object* pinModel);
	void Update();

	Renderable *mSkyBox;
	Projectile *projectile;
	vector<ApexCloth*> cloths;
	vector<Object*> bowlingSets;

	ScreenQuad *mScreen;
	ID3D11DepthStencilView *mZbuffer;
	D3D11_VIEWPORT *mScreenViewport;
	GeometryGenerator *mGeoGen;

	vector<Renderable*>* mRenderables;
	ID3D11Device *mDev;
	ID3D11DeviceContext *mDevcon;
	Apex* mApex;
};

#endif