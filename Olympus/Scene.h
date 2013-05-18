#pragma once

#ifndef SCENE_H
#define SCENE_H

#include "Object.h"
#include "Box.h"
#include "Sphere.h"
#include <vector>
#include "Projectile.h"
#include "CharacterController.h"
#include <string>
class Apex;
//class Camera;

using namespace std;

enum CurrentScene
{
	HUB,
	BOWLING,
	DARKNESS,
	JENGA,
	OPENWORLD
};


class Scene
{
public:
	Scene(	ID3D11Device *dev, ID3D11DeviceContext *devcon, Apex* apex, GeometryGenerator *geoGen,
			Renderable *skyBox,	ScreenQuad *screenQuad, ID3D11DepthStencilView *zbuff, D3D11_VIEWPORT *screenViewport, String sceneName);

	~Scene();

	void Fire(Camera *mCam, float speed);
	void ClearProjectiles();

	void LoadFBX(string filename);
	void LoadPhysX(string filename);
	void LoadSpheres(string filename);
	void LoadSettings(string filename);

	void PlacePins(XMFLOAT3 location, int numlevels, float dist, float scale, Object* pinModel);
	void ResetPins();

	void PlaceJenga(XMFLOAT3 location, int numlevels, float dist, float length);
	void PlaceBlock(XMFLOAT3 location, float length, float width, float height, int side); 
	void ResetJenga();

	void Update();
	void UpdateZbuffers(ID3D11DepthStencilView *zbuffer);
	void UpdateReflective(Camera *cam);

	void ToggleParticles(bool on);

	CharacterController *cController;

	XMFLOAT3 mPinStartPosition;
	float    mDist;
	int	     mNumLevels;

	vector<XMFLOAT3> mJengaStartPosition;
	float    mJengaDist;
	vector<int>	     mJengaNumLevels;
	vector<float>	 mJengaLength;
	float	 mJengaWidth;
	float	 mJengaHeight;
	vector<PxRigidActor*> blocks1;
	vector<PxRigidActor*> blocks2;

	Box* jengaBlock1;
	Box* jengaBlock2;

	Renderable *mSkyBox;
	Projectile *projectile;
	vector<ApexParticles*> particles;
	vector<Sphere*> reflectiveSpheres;
	vector<ApexCloth*> cloths;
	vector<Object*> bowlingSets;
	vector<Box*> JengaBlocks;

	ScreenQuad *mScreen;
	ID3D11DepthStencilView *mZbuffer;
	D3D11_VIEWPORT *mScreenViewport;
	GeometryGenerator *mGeoGen;

	vector<Renderable*> mRenderables;
	vector<Renderable*> mBlendRenderables;
	ID3D11Device *mDev;
	ID3D11DeviceContext *mDevcon;
	Apex* mApex;

	float mGravity;
	int mMaxProjectile;
	float mSpeedScale;

	PointLight mPointLights[21];
};

#endif