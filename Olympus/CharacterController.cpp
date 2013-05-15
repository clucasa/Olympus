#include "CharacterController.h"


const PxF32 CharacterController::minDist = 0.001f;

CharacterController::CharacterController(Apex *mApex)
{
	PxControllerManager* pManager = PxCreateControllerManager(*mApex->mFoundation);

	PxMaterial* mMaterial		= mApex->defaultMaterial;

	PxCapsuleControllerDesc desc;
		desc.height					= 4.0f;
		desc.radius					= 2.0f;
		//desc.halfHeight				= 10.0f; //for box
		desc.density				= 1000.0f;
		desc.scaleCoeff				= 0.899998f;
		desc.material				= mMaterial;
		desc.position				= PxExtendedVec3(0.0, 58.0f, 0.0f);
		desc.stepOffset				= 0.05f;
		//desc.maxJumpHeight			= 20.0f;
		desc.callback				= 0;
		desc.behaviorCallback		= 0;
		desc.upDirection			= PxVec3( 0.0f, 1.0f, 0.0f );
		desc.slopeLimit				= cosf(0.712f);

		pCharacter = pManager->createController( *mApex->mPhysics, mApex->mScene[mApex->mCurrentScene], desc );
	
	PxExtendedVec3 pos = desc.position;//PxExtendedVec3(0.0f, 8.2f, 0.0f);
	pos.y = pCharacter->getFootPosition().y;
	pCharacter->setPosition( pos );

	fpos = pCharacter->getFootPosition();
	gJump = false;
	gJumpTime = 0.0f;
	gV0	= 0.0f;
	jumpHappened = false;
	times = 0.0;
	time_t timer;
	runFlag = false;
	mCurrentScene = 0;
}

CharacterController::~CharacterController(void)
{
	pCharacter->release();
}

void CharacterController::Move( float x, float y, float z, float elapsedTime )
{
	elapsedTime = elapsedTime * 15;

	if (gJump){
		float Vt = gV0 + -9.8*gJumpTime; // Vt = Vo + GT
		gJumpTime += elapsedTime;
		y = Vt*elapsedTime + 1/2*-9.8*elapsedTime*elapsedTime; //maybe: y +=

		double newTimes = time(&timer);
		double diff = difftime(newTimes, times);

		if(diff > 1.){
			gJump = false;
			jumpHappened = false;
			gV0 = 60.0f;
			gJumpTime = 0.0f;
		}
	}

	if(runFlag){
		PxVec3 moveVec( x*2.0f, y, z*2.0f );
		static PxControllerFilters filters( 0 );
		pCharacter->move( moveVec/5, 0.01f, elapsedTime, filters );
	}

	if(zoomFlag){
		PxVec3 moveVec( x/3.0f, y, z/3.0f );
		static PxControllerFilters filters( 0 );
		pCharacter->move( moveVec/5, 0.01f, elapsedTime, filters );
	}
	else{
		PxVec3 moveVec( x, y, z );
		static PxControllerFilters filters( 0 );
		pCharacter->move( moveVec/5, 0.01f, elapsedTime, filters );
	}
}

void CharacterController::zoom(bool zflag)
{
	zoomFlag = zflag;
}

void CharacterController::run(bool rflag)
{
	runFlag = rflag;
}

void CharacterController::StartJump()
{
	if(gJump)  return;
	gJumpTime = 0.0f;
	gV0	= 60.0f;
	gJump = true;
}

void CharacterController::control(bool move, bool startJump, float x, float y, float z, float elapsedTime)
{
	if(!jumpHappened){
		if(startJump){
			times = time(&timer);
			jumpHappened = true;
			StartJump();
		}
	}

	if(move){
		Move(x, y, z, elapsedTime);
	}
}

void CharacterController::MoveTo( float x, float y, float z )
{
	PxExtendedVec3 pos = PxExtendedVec3(x, y, z);
	pCharacter->setPosition( pos );
}