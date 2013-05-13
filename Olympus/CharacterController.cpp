#include "CharacterController.h"


const PxF32 CharacterController::minDist = 0.001f;

CharacterController::CharacterController(Apex *mApex)
{
    PxControllerManager* pManager = PxCreateControllerManager(*mApex->mFoundation);

    PxMaterial* mMaterial		= mApex->defaultMaterial;

    PxCapsuleControllerDesc desc;
        desc.height					= 5.0f;
        desc.radius					= 2.0f;
        //desc.halfHeight				= 10.0f; //for box
        desc.density				= 10.0f;
        desc.scaleCoeff				= 0.899998f;
        desc.material				= mMaterial;
        desc.position				= PxExtendedVec3(0.0f, 0.0f, 0.0);
        desc.stepOffset				= 0.05f;
        //desc.maxJumpHeight			= 20.0f;
        desc.callback				= 0;
        desc.behaviorCallback		= 0;
        desc.upDirection			= PxVec3( 0.0f, 1.0f, 0.0f );
        desc.slopeLimit				= cosf(0.712f);

    pCharacter = pManager->createController( *mApex->mPhysics, mApex->mScene[mApex->mCurrentScene], desc );
    
    pCharacter->setStepOffset( 5.0f*2.0f );
    PxExtendedVec3 pos = PxExtendedVec3(0.0f, 8.2f, 0.0f);
    pos.y = pCharacter->getFootPosition().y;
    pCharacter->setPosition( pos );

    fpos = pCharacter->getFootPosition();
    gJump = false;
    gJumpTime = 0.0f;
    gV0	= 0.0f;
    jumpHappened = false;
    times = 0.0;
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
        float Vt = gV0 + -9.8f*gJumpTime; // Vt = Vo + GT
        gJumpTime += elapsedTime;
        y = Vt*elapsedTime + 1.f/2.f * -9.8f * elapsedTime*elapsedTime; //maybe: y +=

        double newTimes = (double)time(&timer);
        double diff = difftime((time_t)newTimes, (time_t)times);

        if(diff > 1.f){
            gJump = false;
            jumpHappened = false;
            gV0 = 60.0f;
            gJumpTime = 0.0f;
        }
    }


    if(zoomFlag){
        PxVec3 moveVec( x/3.0f, y, z/3.0f );
        static PxControllerFilters filters( 0 );
        pCharacter->move( moveVec/7, 0.01f, elapsedTime, filters );
    }
    else{
        PxVec3 moveVec( x, y, z );
        static PxControllerFilters filters( 0 );
        pCharacter->move( moveVec/7, 0.01f, elapsedTime, filters );
    }
}

void CharacterController::zoom(bool flag)
{
    zoomFlag = flag;
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
            times = (double)time(&timer);
            jumpHappened = true;
            StartJump();
        }
    }

    if(move){
        Move(x, y, z, elapsedTime);
    }
}
