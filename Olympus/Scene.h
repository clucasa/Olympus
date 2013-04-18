#pragma once

#ifndef SCENE_H
#define SCENE_H

#include "Object.h"
#include <vector>

class Apex;

using namespace std;

class Scene
{
public:
	Scene(vector<Renderable*>* renderables,ID3D11Device *dev, ID3D11DeviceContext *devcon, Apex* apex);
	~Scene();
};

#endif