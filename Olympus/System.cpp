#include "System.h"

System* gSystem = 0;            // needed for callback in a class
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return gSystem->msgProc(hWnd, message, wParam, lParam);
}

System::System(HINSTANCE hInstance, int nCmdShow) :
	mAppPaused(false)
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

    devcon->RSSetViewports(1, &viewport);

    
	// initialize camera
	mCam = new Camera();
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	

    mApex = new Apex();
    mApex->Init(dev, devcon);
    mApex->InitParticles();

	rendManager = new RenderManager(devcon, dev, swapchain, mApex, mCam);
    return InitPipeline();
}

struct SPRITECBUFFER
{
	XMMATRIX Final;
	XMFLOAT3 EyePos;
	float buffer;
};

// this is the function used to render a single frame
void System::RenderFrame(float dt)
{
    bool fetch = mApex->advance(dt);

	// Other animation?
	UpdateCamera(dt);

	if(fetch)
        mApex->fetch();

    // set the shader objects
    rendManager->Render();



	//SPRITECBUFFER scBuffer;
	//scBuffer.Final = mCam->ViewProj();
	//scBuffer.EyePos = mCam->GetPosition();

    // Draw ground
    
    //devcon->PSSetShaderResources(0, 1, &groundTexture);


    // Draw sprites
	//devcon->VSSetShader(spVS, 0, 0);
 //   devcon->GSSetShader(spGS, 0, 0);
 //   devcon->PSSetShader(spPS, 0, 0);
	//devcon->IASetInputLayout(spLayout);
	//devcon->GSSetConstantBuffers(0, 1, &spCBuffer);
 //   devcon->PSSetShaderResources(0, 1, &spriteTexture);
	//devcon->UpdateSubresource(spCBuffer, 0, 0, &scBuffer, 0, 0);
	////mApex->Render();
 //   devcon->GSSetShader(NULL, 0, 0);

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
    //// load and compile the two shaders
    //ID3D10Blob *VS, *PS;
    //D3DX11CompileFromFile("Skybox.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    //D3DX11CompileFromFile("Skybox.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);

    //// encapsulate both shaders into shader objects
    //hr = dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
    //if( FAILED(hr) )
    //    return 0;

    //hr = dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);
    //if( FAILED(hr) )
    //    return 0;
    //
    //// create the input layout object
    //D3D11_INPUT_ELEMENT_DESC ied[] =
    //{
    //    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
    //};

    //hr = dev->CreateInputLayout(ied, 4, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
    //if( FAILED(hr) )
    //    return 0;


	// compile the shaders
 //   ID3D10Blob *gVS, *gPS, *sVS, *sPS, *sGS;
 //   HRESULT result = D3DX11CompileFromFile("shaders.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &gVS, 0, 0);
 //   result =  D3DX11CompileFromFile("shaders.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &gPS, 0, 0);
	//
	//result = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &sVS, 0, 0);

 //   result = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "GShader", "gs_5_0", 0, 0, 0, &sGS, 0, 0);

 //   result = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &sPS, 0, 0);


 //   // create the shader objects
 //   dev->CreateVertexShader(gVS->GetBufferPointer(), gVS->GetBufferSize(), NULL, &groundVS);
 //   dev->CreatePixelShader(gPS->GetBufferPointer(), gPS->GetBufferSize(), NULL, &groundPS);

 //   dev->CreateVertexShader(sVS->GetBufferPointer(), sVS->GetBufferSize(), NULL, &spVS);
 //   dev->CreateGeometryShader(sGS->GetBufferPointer(), sGS->GetBufferSize(), NULL, &spGS);
 //   dev->CreatePixelShader(sPS->GetBufferPointer(), sPS->GetBufferSize(), NULL, &spPS);


 //   // create the input element object
 //   D3D11_INPUT_ELEMENT_DESC gied[] =
 //   {
 //       {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
 //       {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
 //       {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
 //   };

	//// create the input element object
 //   D3D11_INPUT_ELEMENT_DESC spriteied[] =
 //   {
 //       {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	//};

 //   // use the input element descriptions to create the input layout
 //   dev->CreateInputLayout(gied, 3, gVS->GetBufferPointer(), gVS->GetBufferSize(), &groundLayout);

 //   dev->CreateInputLayout(spriteied, 1, sVS->GetBufferPointer(), sVS->GetBufferSize(), &spLayout);


 //   // create the constant buffer
 //   D3D11_BUFFER_DESC bd;
 //   ZeroMemory(&bd, sizeof(bd));

 //   bd.Usage = D3D11_USAGE_DEFAULT;
 //   bd.ByteWidth = 176;
 //   bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

 //   dev->CreateBuffer(&bd, NULL, &groundCBuffer);

	//// Create sprite constant buffer
 //   ZeroMemory(&bd, sizeof(bd));

 //   bd.Usage = D3D11_USAGE_DEFAULT;
 //   bd.ByteWidth = 80;    // 4 for each float, float 4x4 = 4 * 4 * 4 + float 3 eyepos
 //   bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

 //   result = dev->CreateBuffer(&bd, NULL, &spCBuffer);

 //   HRESULT hr = D3DX11CreateShaderResourceViewFromFile(dev, "Media/Textures/SoftParticle.dds", 0, 0, &spriteTexture, 0 );


    return 1;
}



/////////////////////////////////////
// CAMERA STUFF
/////////////////////////////////////

void System::UpdateCamera(float dt)
{
	float speed = 10.0f;

    ShowCursor(true);

    if( GetAsyncKeyState(0x10) & 0x8000 )
        speed = 100.0f;

    if( GetAsyncKeyState('W') & 0x8000 )
        mCam->Walk(speed*dt);

    if( GetAsyncKeyState('S') & 0x8000 )
        mCam->Walk(-speed*dt);

    if( GetAsyncKeyState('A') & 0x8000 )
        mCam->Strafe(-speed*dt);

    if( GetAsyncKeyState('D') & 0x8000 )
        mCam->Strafe(speed*dt);

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

