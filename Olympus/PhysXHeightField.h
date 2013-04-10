//***************************************************************************************
// PhysXHeightField.h
//
//
// 
//
//***************************************************************************************
#ifndef PHYSX_HEIGHTFIELD_H
#define PHYSX_HEIGHTFIELD_H
#include "apex.h"
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;
using namespace physx;

struct Heightfield
{
	PxU16 width;
	PxU16 height;
	PxHeightFieldSample* data;
};

class PhysXHeightfield
{
public:
	PhysXHeightfield();
	~PhysXHeightfield();

	void InitHeightfield(PxPhysics* physics, PxScene* scene, const char* filename);

private:
	bool LoadHeightfield(const char* filename);
};

#endif