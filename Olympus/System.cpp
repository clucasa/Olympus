#include "System.h"

System* gSystem = 0;            // needed for callback in a class
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return gSystem->msgProc(hWnd, message, wParam, lParam);
}

System::System(HINSTANCE hInstance, int nCmdShow) :
	mAppPaused(false), mFlyMode(false), mFovFlag(1)
{
    gSystem = this;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize       = sizeof(WNDCLASSEX);
    wc.style        = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc  = WindowProc;
    wc.hInstance    = hInstance;
    wc.hIcon        = ::LoadIcon(hInstance, "ZeusIcon");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "WindowClass";

    RegisterClassEx(&wc);

    RECT wr = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(NULL,
                          "WindowClass",
                          "Olympus Engine",
                          WS_OVERLAPPEDWINDOW,
                          200,
                          100,
                          wr.right - wr.left,
                          wr.bottom - wr.top,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hWnd, nCmdShow);

}

System::~System() {}

int System::init()
{
    return initd3d();
}

int System::run()
{
    // enter the main loop:
    MSG msg;

	mTimer.Reset();

    while(TRUE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if(msg.message == WM_QUIT)
                break;
        }
		else
		{
			if( !mAppPaused )
			{
				mTimer.Tick();
				RenderFrame(mTimer.DeltaTime());
			}
			else
			{
				Sleep(100);
			}
		}
    }

    // clean up DirectX and COM
    CleanD3D();

    return msg.wParam;
}


// this is the main message handler for the program
LRESULT System::msgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        // WM_ACTIVATE is sent when the window is activated or deactivated.  
	    // We pause the game when the window is deactivated and unpause it 
	    // when it becomes active.  
	    case WM_ACTIVATE:
		    if( LOWORD(wParam) == WA_INACTIVE )
		    {
			    mAppPaused = true;
			    mTimer.Stop();
		    }
		    else
		    {
			    mAppPaused = false;
			    mTimer.Start();
		    }
		    return 0;

	
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;

		case WM_RBUTTONDOWN:
		    OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

		case WM_RBUTTONUP:
		    OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

		case WM_MOUSEMOVE:
		    OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

        case WM_KEYUP:
	    {
		    switch(wParam)
		    {
			    case 0x1B: // Esc key has been pressed
                {
				    PostQuitMessage(0);	
				    return 0;
                }
			    break;
		    }
	    }
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}


// this function initializes and prepares Direct3D for use
int System::initd3d()
{
	// create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC scd;

    // clear out the struct for use
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    // fill the swap chain description struct
    scd.BufferCount = 1;                                   // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // use 32-bit color
    scd.BufferDesc.Width = SCREEN_WIDTH;                   // set the back buffer width
    scd.BufferDesc.Height = SCREEN_HEIGHT;                 // set the back buffer height
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;     // how swap chain is to be used
    scd.OutputWindow = hWnd;                               // the window to be used
    scd.SampleDesc.Count = 4;                              // how many multisamples
    scd.Windowed = TRUE;                                   // windowed/full-screen mode
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;    // allow full-screen switching
	
    // create a device, device context and swap chain using the information in the scd struct
    hr = D3D11CreateDeviceAndSwapChain(NULL,
                                  D3D_DRIVER_TYPE_HARDWARE,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  D3D11_SDK_VERSION,
                                  &scd,
                                  &swapchain,
                                  &dev,
                                  NULL,
                                  &devcon);

    if( FAILED(hr) )
        return 0;

    // Set the viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = SCREEN_WIDTH;
    viewport.Height = SCREEN_HEIGHT;
	viewport.MaxDepth = 1;
	viewport.MinDepth = 0;
	
    devcon->RSSetViewports(1, &viewport);

    
	// initialize camera
	mCam = new Camera();
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;
    mCam->SetPosition(0.0f, 5.0f, -10.0f);
	

    mApex = new Apex();
    mApex->Init(dev, devcon);
    mApex->InitParticles();

	rendManager = new RenderManager(devcon, dev, swapchain, mApex, mCam);
    return InitPipeline();
}


float timePassed = 0.0f;
// this is the function used to render a single frame
void System::RenderFrame(float dt)
{
	
	bool fetch = mApex->advance(dt);

	
	timePassed += dt;
	// Other animation?
	float x,y,z;
	x = 20.f * (float)sin((float)timePassed);
	y = abs(20.f * (float)sin((float)timePassed/1.33f));
	z = 20.f * (float)cos((float)timePassed);

	rendManager->SetPosition(x,y,z);
	UpdateCamera(dt);

	if(fetch)
        mApex->fetch();

    // set the shader objects
    rendManager->Render();


    swapchain->Present(0, 0);

	//D3DX11SaveTextureToFile(devcon, rendManager->mDepthTargetTexture, D3DX11_IFF_JPG, "test.jpg");
}


// this is the function that cleans up Direct3D and COM
void System::CleanD3D(void)
{
    swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

    // close and release all existing COM objects
    swapchain->Release();
    dev->Release();
    devcon->Release();
}


// this function loads and prepares the shaders
int System::InitPipeline()
{
    return 1;
}



/////////////////////////////////////
// CAMERA STUFF
/////////////////////////////////////

void System::UpdateCamera(float dt)
{
	//////////////////////////////////
    //    XINPUT Camera Controls    //
    //////////////////////////////////

    DWORD dwResult;
    XINPUT_STATE state;
 
    ZeroMemory( &state, sizeof(XINPUT_STATE) );
 
    dwResult = XInputGetState( 0, &state );
 
    if( dwResult == ERROR_SUCCESS ){ // Controller is connected.
        float speed = 1.0f;
        if( state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB )
            speed = 2.0f;
        
		ShowCursor(false);

        // Check to make sure we are not moving during the dead zone
        if((state.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
            state.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
           (state.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
            state.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)){    
                state.Gamepad.sThumbLX = 0;
                state.Gamepad.sThumbLY = 0;
        }
 
        if((state.Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
            state.Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) &&
           (state.Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
            state.Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)){
                state.Gamepad.sThumbRX = 0;
                state.Gamepad.sThumbRY = 0;
        }

        float leftThumbY = state.Gamepad.sThumbLY;
        float leftThumbX = state.Gamepad.sThumbLX;
        float rightThumbY = state.Gamepad.sThumbRY;
        float rightThumbX = state.Gamepad.sThumbRX;

		if(mFovFlag == 1){
			mCam->SetLens(0.25f*MathHelper::Pi, 1.0f, 1.0f, 1000.0f);
			mFovFlag = 0;
		}

        // Aiming with left trigger
        if(state.Gamepad.bLeftTrigger && state.Gamepad.bRightTrigger < 256){ // 256 disables the right trigger
			mCam->SetLens(0.1f*MathHelper::Pi, 1.0f, 1.0f, 1000.0f);
			mFovFlag = 1;
            mCam->Walk((leftThumbY / 30000.0f) * dt * speed);
            mCam->Strafe((leftThumbX / 30000.0f) * dt * speed);
            mCam->Pitch((-rightThumbY / 102000.0f) * dt);
            mCam->RotateY((rightThumbX / 105000.0f) * dt);
        }
        else{
            mCam->Walk((leftThumbY / 3000.0f) * dt * speed);
            mCam->Strafe((leftThumbX / 3000.0f) * dt * speed);
			
			XMFLOAT3 mLook = mCam->GetLook();
			if(mLook.y < -0.98){ // looking down limit
				if(-rightThumbY < 0){
					mCam->Pitch((-rightThumbY / 12000.0f) * dt);
				}
			}
			else if(mLook.y > 0.98){ // looking up limit
				if(-rightThumbY > 0){
					mCam->Pitch((-rightThumbY / 12000.0f) * dt);
				}
			}
			else{
		        mCam->Pitch((-rightThumbY / 12000.0f) * dt);
			}

			mCam->RotateY((rightThumbX / 8500.0f) * dt);
        }
		XMFLOAT3 camPos = mCam->GetPosition();
		if(mFlyMode == false){
			
			mCam->SetPosition(camPos.x, 2.0f, camPos.z);
	
			//crouch
			if(state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
				mCam->SetPosition(camPos.x, 1.0f, camPos.z);

			//jump
			if(state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
				mCam->SetPosition(camPos.x, 5.0f, camPos.z);
		}
		else{
			mCam->SetPosition(camPos.x, camPos.y, camPos.z);
		}

        /*// Shoot block with right trigger     
        if( state.Gamepad.bRightTrigger && state.Gamepad.bLeftTrigger < 256 )
        {
            shootspeed = (state.Gamepad.bRightTrigger / 255) * 40.0f;
            if(shootBox)
            {
                mPhysX->CreateBox( mCam.GetPosition().x, mCam.GetPosition().y, mCam.GetPosition().z,
					    	       mCam.GetLook().x, mCam.GetLook().y, mCam.GetLook().z, shootspeed);
            }
            else
            {
                mPhysX->CreateBox( 0., 10.0f, 0., 0., 0., 0., 0.);
            }
        }*/
		
    }
    else{ // Controller is disconnected, oh balls
        float speed = 10.0f;
        ShowCursor(true);
        if( GetAsyncKeyState(0x10) & 0x8000 )
            speed = 20.0f;

        if( GetAsyncKeyState('W') & 0x8000 )
            mCam->Walk(speed*dt);

        if( GetAsyncKeyState('S') & 0x8000 )
            mCam->Walk(-speed*dt);

        if( GetAsyncKeyState('A') & 0x8000 )
            mCam->Strafe(-speed*dt);

        if( GetAsyncKeyState('D') & 0x8000 )
            mCam->Strafe(speed*dt);
    }
	

	if(GetAsyncKeyState('F') & 0x8000 ) // Fly Mode
          mFlyMode = true;
	
	if(GetAsyncKeyState('G') & 0x8000 )
		  mFlyMode = false;


	if( GetAsyncKeyState('P') & 0x8000 ){ // Super Zoom
          mCam->SetLens(0.01f*MathHelper::Pi, 1.0f, 1.0f, 1000.0f); 
		  mFovFlag = 1;
	}

	mCam->UpdateViewMatrix();
}

void System::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(hWnd);
}


void System::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}


void System::OnMouseMove(WPARAM btnState, int x, int y)
{
    if( (btnState & MK_LBUTTON) != 0 )
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        mCam->Pitch(dy);
        mCam->RotateY(dx);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

//////////////////////////////////
// END CAMERA STUFF
//////////////////////////////////

