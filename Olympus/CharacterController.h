#pragma once
#include "Apex.h"
#include <math.h>

#pragma comment(lib, "PhysX3CharacterKinematicCHECKED_x86.lib")

class CharacterController
{
public:
	PxController*		pCharacter;

	string				currentPos;
	string				movePos;

	static const PxF32	minDist;

	float				currentDirection;

public:
	PxExtendedVec3 fpos;

	CharacterController(Apex *mApex);
	~CharacterController(void);

	//void setPosition( float x, float y, float z );
	void move( float x, float y, float z, PxF32 elapsedTime );
	void rotateX( float angle ) const;
	void scale( float x, float y, float z ) const;

	D3DVECTOR getPosition();
	void changeDirection( float angle ) const
	{	
		//this->pCharEntity->getNode()->setRotationAngleY( angle );
	}

	void update();

	
};