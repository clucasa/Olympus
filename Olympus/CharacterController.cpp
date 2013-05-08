#include "CharacterController.h"

const PxF32 CharacterController::minDist = 0.001;

CharacterController::CharacterController(Apex *mApex)
{
	PxControllerManager* pManager = PxCreateControllerManager(*mApex->getFoundation());

	PxMaterial* mMaterial		= mApex->getPhysics()->createMaterial(0.5f, 0.5f, 0.1f);
	PxCapsuleControllerDesc desc;
		desc.height					= 10.0f;
		desc.radius					= 3.0f;
		desc.density				= 10.0f;
		desc.scaleCoeff				= 0.899998f;
		desc.material				= mMaterial;
		desc.position				= PxExtendedVec3(0.0f, 5.0f, 0.0);
		desc.stepOffset				= 0.05f;
		desc.maxJumpHeight			= .5f;
		desc.callback				= 0;
		desc.behaviorCallback		= 0;
		desc.upDirection			= PxVec3( 0.0f, 1.0f, 0.0f );
		desc.slopeLimit				= cosf(0.712f);
		desc.invisibleWallHeight	= 700.0f;

		pCharacter = pManager->createController( *mApex->getPhysics(), mApex->getScene(), desc );

	pCharacter->setStepOffset( 5.0f*2.0f );
	PxExtendedVec3 pos = PxExtendedVec3(0.0, 5.0, 0.0);
	pos.y = pCharacter->getFootPosition().y;
	pCharacter->setPosition( pos );

	fpos = pCharacter->getFootPosition();
}


CharacterController::~CharacterController(void)
{
	pCharacter->release();
}


void CharacterController::move( float x, float y, float z, float elapsedTime )
{
	PxVec3 moveVec( x, y, z );
	static PxControllerFilters filters( 0 );
	pCharacter->move( moveVec, 0.01f, elapsedTime, filters );

	const PxExtendedVec3 pos = pCharacter->getFootPosition();
	pCharacter->setPosition(pos);
}


D3DVECTOR CharacterController::getPosition()
{
	fpos = pCharacter->getFootPosition();
	D3DVECTOR newPos = { fpos.x, fpos.y, fpos.z };
	return newPos;
}


void CharacterController::update()
{

}
