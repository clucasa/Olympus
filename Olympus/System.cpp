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
    wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "WindowClass";

    RegisterClassEx(&wc);

    RECT wr = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(NULL,
                          "WindowClass",
                          "Olympus Engine",
                          WS_OVERLAPPEDWINDOW,
                          100,
                          100,
                          wr.right - wr.left,
                          wr.bottom - wr.top,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

	mAppPaused = false;
	mMinimized = false;
	mMaximized = false;
	mResizing  = false;

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

	RenderFrame(100.0f); // skip forward 100 seconds!

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
		case WM_SIZE:
			// Save the new client area dimensions.
			mClientWidth  = LOWORD(lParam);
			mClientHeight = HIWORD(lParam);
			if( dev )
			{
				if( wParam == SIZE_MINIMIZED )
				{
					mAppPaused = true;
					mMinimized = true;
					mMaximized = false;
				}
				else if( wParam == SIZE_MAXIMIZED )
				{
					mAppPaused = false;
					mMinimized = false;
					mMaximized = true;
					OnResize();
				}
				else if( wParam == SIZE_RESTORED )
				{

					// Restoring from minimized state?
					if( mMinimized )
					{
						mAppPaused = false;
						mMinimized = false;
						OnResize();
					}

					// Restoring from maximized state?
					else if( mMaximized )
					{
						mAppPaused = false;
						mMaximized = false;
						OnResize();
					}
					else if( mResizing )
					{
						// If user is dragging the resize bars, we do not resize 
						// the buffers here because as the user continuously 
						// drags the resize bars, a stream of WM_SIZE messages are
						// sent to the window, and it would be pointless (and slow)
						// to resize for each WM_SIZE message received from dragging
						// the resize bars.  So instead, we reset after the user is 
						// done resizing the window and releases the resize bars, which 
						// sends a WM_EXITSIZEMOVE message.
					}
					else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
					{
						OnResize();
					}
				}
			}
			return 0;

			// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
			case WM_ENTERSIZEMOVE:
				mAppPaused = true;
				mResizing  = true;
				mTimer.Stop();
				return 0;

			// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
			// Here we reset everything based on the new window dimensions.
			case WM_EXITSIZEMOVE:
				mAppPaused = false;
				mResizing  = false;
				mTimer.Start();
				OnResize();
				return 0;
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
    scd.SampleDesc.Count = 1;                              // how many multisamples
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
    ZeroMemory(&mViewport, sizeof(D3D11_VIEWPORT));

    mViewport.TopLeftX = 0;
    mViewport.TopLeftY = 0;
    mViewport.Width = SCREEN_WIDTH;
    mViewport.Height = SCREEN_HEIGHT;
	mViewport.MaxDepth = 1;
	mViewport.MinDepth = 0;
	
    devcon->RSSetViewports(1, &mViewport);

    
	// initialize camera
	mCam = new Camera();
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;
    mCam->SetPosition(0.0f, 5.0f, 10.0f);
	//mCam->RotateY(90);
	mCam->SetLens(0.25f*MathHelper::Pi, (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 1.0f, 10000.0f);

    mApex = new Apex();
    mApex->Init(dev, devcon);
    mApex->InitParticles();
    mApex->InitClothing();

	rendManager = new RenderManager(devcon, dev, swapchain, mApex, mCam, &mViewport);
    return InitPipeline();
}


float timePassed = 0.0f;
// this is the function used to render a single frame
void System::RenderFrame(float dt)
{
	mApex->UpdateViewProjMat(&mCam->View(),&mCam->Proj(),1.0f, 10000.0f, 0.25f*MathHelper::Pi, mClientWidth, mClientHeight);
	bool fetch = mApex->advance(dt);

	//rendManager->UpdateShit();
	timePassed += dt;
	// Other animation?
	float x,y,z;
	x = 200.0f * (float)sin((float)timePassed);
	y = abs(50.f * (float)sin((float)timePassed/0.3f))-10.0f;
	z = 200.0f * (float)cos((float)timePassed);

	rendManager->SetPosition(x,y,z);
	rendManager->mSphereMove->MoveTo(x,y,z);
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
            speed = 40.0f;
        
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
			mCam->SetLens(0.25f*MathHelper::Pi, (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 1.0f, 10000.0f);
			mFovFlag = 0;
		}

        // Aiming with left trigger
        if(state.Gamepad.bLeftTrigger && state.Gamepad.bRightTrigger < 256){ // 256 disables the right trigger
			mCam->SetLens(0.1f*MathHelper::Pi, (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 1.0f, 10000.0f);
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

	if(GetAsyncKeyState('R') & 0x8000 )
		rendManager->RecompShaders();

	if( GetAsyncKeyState('P') & 0x8000 ){ // Super Zoom
          mCam->SetLens(0.01f*MathHelper::Pi, (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 1.0f, 10000.0f); 
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


void System::OnResize()
{
	//Set the new aspect ration for the camera
	rendManager->mCam->SetLens(0.25f*MathHelper::Pi, (float)mClientWidth/(float)mClientHeight, 1.0f, 10000.0f);

	//Release anything related to the swapchain

	HRESULT hr;

	hr = rendManager->mBackbuffer->Release();
	//hr = rendManager->mZbuffer->Release();
	//hr = rendManager->mDepthShaderResourceView->Release();


	hr = swapchain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	ID3D11Texture2D* backBuffer;

	hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	hr = dev->CreateRenderTargetView(backBuffer, 0, &rendManager->mBackbuffer);

	backBuffer->Release();

	rendManager->mScreen->mShaderResourceView->Release();
	rendManager->mScreen->mTargetTexture->Release();
	rendManager->mScreen->mTargetView->Release();

	rendManager->mScreen->SetupRenderTarget(mClientWidth, mClientHeight);




	rendManager->mDepthTargetTexture->Release();
	rendManager->mDepthShaderResourceView->Release();
	rendManager->mZbuffer->Release();
	
	D3D11_TEXTURE2D_DESC texd;
	ZeroMemory(&texd, sizeof(texd));

	texd.Width = mClientWidth;
	texd.Height = mClientWidth;
	texd.ArraySize = 1;
	texd.MipLevels = 1;
	texd.SampleDesc.Count = 1;
	texd.Format = DXGI_FORMAT_R32_TYPELESS;
	texd.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	dev->CreateTexture2D(&texd, NULL, &rendManager->mDepthTargetTexture);


	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(dsvd));

	dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;

	dev->CreateDepthStencilView(rendManager->mDepthTargetTexture, &dsvd, &rendManager->mZbuffer);

	rendManager->mSphere->mZbuffer = rendManager->mZbuffer;
	
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	dev->CreateShaderResourceView(rendManager->mDepthTargetTexture, &shaderResourceViewDesc, &rendManager->mDepthShaderResourceView);


	// Set the viewport
    
    ZeroMemory(&mViewport, sizeof(D3D11_VIEWPORT));

    mViewport.TopLeftX = 0;
    mViewport.TopLeftY = 0;
    mViewport.Width = mClientWidth;
    mViewport.Height = mClientHeight;
	mViewport.MaxDepth = 1;
	mViewport.MinDepth = 0;
	
    devcon->RSSetViewports(1, &mViewport);
}
 