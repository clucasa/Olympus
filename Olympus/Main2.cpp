//// include the basic windows header files and the Direct3D header files
//#include <windows.h>
//#include <windowsx.h>
//#include <d3d11.h>
//#include <d3dx11.h>
//#include <d3dx10.h>
//
//#include "apex.h"
//
//// include the Direct3D Library file
//#pragma comment (lib, "d3d11.lib")
//#pragma comment (lib, "d3dx11.lib")
//#pragma comment (lib, "d3dx10.lib")
//
//// define the screen resolution
//#define SCREEN_WIDTH  800
//#define SCREEN_HEIGHT 600
//
//// global declarations
//IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
//ID3D11Device *dev;                     // the pointer to our Direct3D device interface
//ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
//ID3D11RenderTargetView *backbuffer;    // the pointer to our back buffer
//ID3D11DepthStencilView *zbuffer;       // the pointer to our depth buffer
//ID3D11InputLayout *pLayout;            // the pointer to the input layout
//ID3D11VertexShader *pVS;               // the pointer to the vertex shader
//ID3D11PixelShader *pPS;                // the pointer to the pixel shader
//ID3D11Buffer *pVBuffer;                // the pointer to the vertex buffer
//ID3D11Buffer *pIBuffer;                // the pointer to the index buffer
//ID3D11Buffer *pCBuffer;                // the pointer to the constant buffer
//ID3D11ShaderResourceView *pTexture;    // the pointer to the texture
//
//ID3D11InputLayout *pSpriteLayout;            // the pointer to the input layout
//ID3D11VertexShader *pSpriteVS;         // the pointer to the Sprite vertex shader
//ID3D11GeometryShader *pSpriteGS;	   // the pointer to the Sprite geometry shader
//ID3D11PixelShader *pSpritePS;          // the pointer to the Sprite pixel shader
//ID3D11Buffer *pSpriteCBuffer;          // the pointer to the Sprite constant buffer
//
//D3DXMATRIX mMatProjection;
//D3DXMATRIX mMatView;
//
//// a struct to define a single vertex
//struct VERTEX {FLOAT X, Y, Z; D3DXVECTOR3 Normal; FLOAT U, V;};
//
//// a struct to define the constant buffer
//struct CBUFFER
//{
//    D3DXMATRIX Final;
//    D3DXMATRIX Rotation;
//    D3DXVECTOR4 LightVector;
//    D3DXCOLOR LightColor;
//    D3DXCOLOR AmbientColor;
//};
//
//struct SPRITECBUFFER
//{
//	D3DXMATRIX Final;
//	D3DXVECTOR3 EyePos;
//	float buffer;
//};
//
//struct CAMERA
//{
//	float x,  y, z;
//	float vx, vy,   vz;
//	float rx, ry,   rz;
//	bool moving;
//};
//
//struct CAMERA mCam;
//
//// function prototypes
//void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
//void RenderFrame(void);     // renders a single frame
//void CleanD3D(void);        // closes Direct3D and releases memory
//void InitGraphics(void);    // creates the shape to render
//void InitPipeline(void);    // loads and prepares the shaders
//bool InitApex(ID3D11DeviceContext* devcon, ID3D11Device* dev);
//void RecalculateView();
//
//// the WindowProc function prototype
//LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
//
//
//// the entry point for any Windows program
//int WINAPI WinMain(HINSTANCE hInstance,
//                   HINSTANCE hPrevInstance,
//                   LPSTR lpCmdLine,
//                   int nCmdShow)
//{
//    HWND hWnd;
//    WNDCLASSEX wc;
//
//    ZeroMemory(&wc, sizeof(WNDCLASSEX));
//
//    wc.cbSize = sizeof(WNDCLASSEX);
//    wc.style = CS_HREDRAW | CS_VREDRAW;
//    wc.lpfnWndProc = WindowProc;
//    wc.hInstance = hInstance;
//    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
//    wc.lpszClassName = "WindowClass";
//
//    RegisterClassEx(&wc);
//
//    RECT wr = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
//    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
//
//    hWnd = CreateWindowEx(NULL,
//                          "WindowClass",
//                          "Our First Direct3D Program",
//                          WS_OVERLAPPEDWINDOW,
//                          300,
//                          300,
//                          wr.right - wr.left,
//                          wr.bottom - wr.top,
//                          NULL,
//                          NULL,
//                          hInstance,
//                          NULL);
//
//    ShowWindow(hWnd, nCmdShow);
//
//    // set up and initialize Direct3D
//    InitD3D(hWnd);
//
//    // enter the main loop:
//
//    MSG msg;
//
//    while(TRUE)
//    {
//        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
//        {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//
//            if(msg.message == WM_QUIT)
//                break;
//        }
//
//        RenderFrame();
//    }
//
//    // clean up DirectX and COM
//    CleanD3D();
//
//    return msg.wParam;
//}
//
//
//// this is the main message handler for the program
//LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	float movement = 1.0f;
//    switch(message)
//    {
//        case WM_DESTROY:
//            {
//                PostQuitMessage(0);
//                return 0;
//            } break;
//		case WM_KEYDOWN:
//			{
//				switch(wParam)
//				{
//				case 0x57: // W
//					mCam.z -= movement;
//					break;
//				case 0x53: // S
//					mCam.z += movement;
//					break; 
//				case 0x41: // A
//					mCam.x -= movement;
//					break;
//				case 0x44: // D
//					mCam.x += movement;
//					break;
//				}
//			} break;
//		case WM_KEYUP:
//			{
//				switch(wParam)
//				{
//
//				}
//			} break;
//    }
//
//    return DefWindowProc (hWnd, message, wParam, lParam);
//}
//
//
//// this function initializes and prepares Direct3D for use
//void InitD3D(HWND hWnd)
//{
//    // create a struct to hold information about the swap chain
//    DXGI_SWAP_CHAIN_DESC scd;
//
//    // clear out the struct for use
//    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
//
//    // fill the swap chain description struct
//    scd.BufferCount = 1;                                   // one back buffer
//    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // use 32-bit color
//    scd.BufferDesc.Width = SCREEN_WIDTH;                   // set the back buffer width
//    scd.BufferDesc.Height = SCREEN_HEIGHT;                 // set the back buffer height
//    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;     // how swap chain is to be used
//    scd.OutputWindow = hWnd;                               // the window to be used
//    scd.SampleDesc.Count = 4;                              // how many multisamples
//    scd.Windowed = TRUE;                                   // windowed/full-screen mode
//    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;    // allow full-screen switching
//
//    // create a device, device context and swap chain using the information in the scd struct
//    D3D11CreateDeviceAndSwapChain(NULL,
//                                  D3D_DRIVER_TYPE_HARDWARE,
//                                  NULL,
//                                  NULL,
//                                  NULL,
//                                  NULL,
//                                  D3D11_SDK_VERSION,
//                                  &scd,
//                                  &swapchain,
//                                  &dev,
//                                  NULL,
//                                  &devcon);
//
//
//    // create the depth buffer texture
//    D3D11_TEXTURE2D_DESC texd;
//    ZeroMemory(&texd, sizeof(texd));
//
//    texd.Width = SCREEN_WIDTH;
//    texd.Height = SCREEN_HEIGHT;
//    texd.ArraySize = 1;
//    texd.MipLevels = 1;
//    texd.SampleDesc.Count = 4;
//    texd.Format = DXGI_FORMAT_D32_FLOAT;
//    texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
//
//    ID3D11Texture2D *pDepthBuffer;
//    dev->CreateTexture2D(&texd, NULL, &pDepthBuffer);
//
//    // create the depth buffer
//    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
//    ZeroMemory(&dsvd, sizeof(dsvd));
//
//    dsvd.Format = DXGI_FORMAT_D32_FLOAT;
//    dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
//
//    dev->CreateDepthStencilView(pDepthBuffer, &dsvd, &zbuffer);
//    pDepthBuffer->Release();
//
//    // get the address of the back buffer
//    ID3D11Texture2D *pBackBuffer;
//    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
//
//    // use the back buffer address to create the render target
//    dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
//    pBackBuffer->Release();
//
//    // set the render target as the back buffer
//    devcon->OMSetRenderTargets(1, &backbuffer, zbuffer);
//
//
//    // Set the viewport
//    D3D11_VIEWPORT viewport;
//    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
//
//    viewport.TopLeftX = 0;    // set the left to 0
//    viewport.TopLeftY = 0;    // set the top to 0
//    viewport.Width = SCREEN_WIDTH;    // set the width to the window's width
//    viewport.Height = SCREEN_HEIGHT;    // set the height to the window's height
//    viewport.MinDepth = 0;    // the closest an object can be on the depth buffer is 0.0
//    viewport.MaxDepth = 1;    // the farthest an object can be on the depth buffer is 1.0
//
//    devcon->RSSetViewports(1, &viewport);
//	
//	mCam.x = 0.0f;
//	mCam.y = 1.5f;
//	mCam.z = -10.0f;
//	mCam.vx = mCam.vy = mCam.vz = mCam.rx = mCam.ry = mCam.rz = 0.0f;
//	mCam.moving = false;
//    
//	InitPipeline();
//    InitGraphics();
//}
//
//Apex* apexThisOne;
//// this is the function used to render a single frame
//void RenderFrame(void)
//{
//    bool fetch = apexThisOne->advance(0.0003f);
//    CBUFFER cBuffer;
//	SPRITECBUFFER scBuffer;
//
//
//    cBuffer.LightVector = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 0.0f);
//    cBuffer.LightColor = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
//    cBuffer.AmbientColor = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
//
//    D3DXMATRIX matRotate, matTrans;
//    D3DXMATRIX matFinal;
//
//    static float Time = 0.0f; Time += 0.0003f;
//
//    // create a world matrices
//    D3DXMatrixRotationY(&matRotate, /*Time/2.0f*/0);
//
//	// move plane down
//	D3DXMatrixTranslation(&matTrans,0., -5.0f,0);
//
//	RecalculateView();
//
//    // load the matrices into the constant buffer
//    cBuffer.Final = matRotate * matTrans * mMatView * mMatProjection;
//    cBuffer.Rotation = matRotate;
//
//	scBuffer.Final = mMatView * mMatProjection;
//	scBuffer.EyePos = D3DXVECTOR3(mCam.x, mCam.y, mCam.z);
//
//    // clear the back buffer to a deep blue
//    devcon->ClearRenderTargetView(backbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));
//
//    // clear the depth buffer
//    devcon->ClearDepthStencilView(zbuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
//
//    // select which vertex buffer to display
//    UINT stride = sizeof(VERTEX);
//    UINT offset = 0;
//    devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
//    devcon->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);
//
//    // select which primtive type we are using
//    devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//    // set the shader objects
//    devcon->VSSetShader(pVS, 0, 0);
//	devcon->GSSetShader(NULL, 0, 0);
//    devcon->PSSetShader(pPS, 0, 0);
//	devcon->IASetInputLayout(pLayout);
//	devcon->VSSetConstantBuffers(0, 1, &pCBuffer);
//
//	// draw the floor
//    devcon->UpdateSubresource(pCBuffer, 0, 0, &cBuffer, 0, 0);
//    devcon->PSSetShaderResources(0, 1, &pTexture);
//    devcon->DrawIndexed(36, 0, 0);
//
//    if(fetch)
//        apexThisOne->fetch();
//
//	// set the sprite shader objects
//    devcon->VSSetShader(pSpriteVS, 0, 0);
//    devcon->GSSetShader(pSpriteGS, 0, 0);
//    devcon->PSSetShader(pSpritePS, 0, 0);
//	devcon->IASetInputLayout(pSpriteLayout);
//	//devcon->VSSetConstantBuffers(0, 1, &pSpriteCBuffer);
//	devcon->GSSetConstantBuffers(0, 1, &pSpriteCBuffer);
//
//	
//	devcon->UpdateSubresource(pSpriteCBuffer, 0, 0, &scBuffer, 0, 0);
//
//    apexThisOne->Render();
//
//    // switch the back buffer and the front buffer
//    swapchain->Present(0, 0);
//}
//
//
//// this is the function that cleans up Direct3D and COM
//void CleanD3D(void)
//{
//    swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode
//
//    // close and release all existing COM objects
//    zbuffer->Release();
//    pLayout->Release();
//    pVS->Release();
//    pPS->Release();
//    pVBuffer->Release();
//    pIBuffer->Release();
//    pCBuffer->Release();
//	pSpriteLayout->Release(); 
//	pSpriteVS->Release();
//	pSpriteGS->Release();
//	pSpritePS->Release(); 
//	pSpriteCBuffer->Release();
//    pTexture->Release();
//    swapchain->Release();
//    backbuffer->Release();
//    dev->Release();
//    devcon->Release();
//}
//
//
//// this is the function that creates the shape to render
//void InitGraphics()
//{
//	float size = 15.0f;
//    // create vertices to represent the corners of the cube
//    VERTEX OurVertices[] =
//    {
//
//        {-size, 0.0f, -size, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f},    // side 4
//        {size, 0.0f, -size, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0.0f, 1.0f},
//        {-size, 0.0f, size, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 1.0f, 0.0f},
//        {size, 0.0f, size, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 1.0f, 1.0f},
//
//    };
//
//    // create the vertex buffer
//    D3D11_BUFFER_DESC bd;
//    ZeroMemory(&bd, sizeof(bd));
//
//    bd.Usage = D3D11_USAGE_DYNAMIC;
//    bd.ByteWidth = sizeof(VERTEX) * 4;
//    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//
//    dev->CreateBuffer(&bd, NULL, &pVBuffer);
//
//    // copy the vertices into the buffer
//    D3D11_MAPPED_SUBRESOURCE ms;
//    devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
//    memcpy(ms.pData, OurVertices, sizeof(OurVertices));                 // copy the data
//    devcon->Unmap(pVBuffer, NULL);
//
//
//    // create the index buffer out of DWORDs
//    DWORD OurIndices[] = 
//    {
//        2, 1, 0,    // quad
//        3, 1, 2,
//    };
//
//    // create the index buffer
//    bd.Usage = D3D11_USAGE_DYNAMIC;
//    bd.ByteWidth = sizeof(DWORD) * 6;
//    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//    bd.MiscFlags = 0;
//
//    dev->CreateBuffer(&bd, NULL, &pIBuffer);
//
//    devcon->Map(pIBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
//    memcpy(ms.pData, OurIndices, sizeof(OurIndices));                   // copy the data
//    devcon->Unmap(pIBuffer, NULL);
//
//    D3DX11CreateShaderResourceViewFromFile(dev,        // the Direct3D device
//                                       "Wood.png",    // load Wood.png in the local folder
//                                       NULL,           // no additional information
//                                       NULL,           // no multithreading
//                                       &pTexture,      // address of the shader-resource-view
//                                       NULL);          // no multithreading
//
//
//    InitApex(devcon, dev);
//	
//	// create a projection matrix
//    D3DXMatrixPerspectiveFovLH(&mMatProjection,
//                               (FLOAT)D3DXToRadian(45),                    // field of view
//                               (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // aspect ratio
//                               1.0f,                                       // near view-plane
//                               100.0f);                                    // far view-plane
//
//	// create a view matrix
//    D3DXMatrixLookAtLH(&mMatView,
//                       &D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // the camera position
//                       &D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // the look-at position
//                       &D3DXVECTOR3(0.0f, 1.0f, 0.0f));   // the up direction
//}
//
//
//// this function loads and prepares the shaders
//void InitPipeline()
//{
//    // compile the shaders
//    ID3D10Blob *VS, *PS, *sVS, *sPS, *sGS;
//    HRESULT result = D3DX11CompileFromFile("shaders.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
//    result =  D3DX11CompileFromFile("shaders.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);
//	
//	result = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &sVS, 0, 0);
//
//    result = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "GShader", "gs_5_0", 0, 0, 0, &sGS, 0, 0);
//
//    result = D3DX11CompileFromFile("spriteshader.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &sPS, 0, 0);
//
//
//    // create the shader objects
//    dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
//    dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);
//
//    dev->CreateVertexShader(sVS->GetBufferPointer(), sVS->GetBufferSize(), NULL, &pSpriteVS);
//    dev->CreateGeometryShader(sGS->GetBufferPointer(), sGS->GetBufferSize(), NULL, &pSpriteGS);
//    dev->CreatePixelShader(sPS->GetBufferPointer(), sPS->GetBufferSize(), NULL, &pSpritePS);
//
//
//    // create the input element object
//    D3D11_INPUT_ELEMENT_DESC ied[] =
//    {
//        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
//        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
//        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
//    };
//
//	// create the input element object
//    D3D11_INPUT_ELEMENT_DESC spriteied[] =
//    {
//        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
//	};
//
//    // use the input element descriptions to create the input layout
//    dev->CreateInputLayout(ied, 3, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
//
//    dev->CreateInputLayout(spriteied, 1, sVS->GetBufferPointer(), sVS->GetBufferSize(), &pSpriteLayout);
//
//
//    // create the constant buffer
//    D3D11_BUFFER_DESC bd;
//    ZeroMemory(&bd, sizeof(bd));
//
//    bd.Usage = D3D11_USAGE_DEFAULT;
//    bd.ByteWidth = 176;
//    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//
//    dev->CreateBuffer(&bd, NULL, &pCBuffer);
//
//	// Create sprite constant buffer
//    ZeroMemory(&bd, sizeof(bd));
//
//    bd.Usage = D3D11_USAGE_DEFAULT;
//    bd.ByteWidth = 80;    // 4 for each float, float 4x4 = 4 * 4 * 4 + float 3 eyepos
//    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//
//    result = dev->CreateBuffer(&bd, NULL, &pSpriteCBuffer);
//}
//
//bool InitApex(ID3D11DeviceContext* devcon, ID3D11Device* dev)
//{
//    apexThisOne = new Apex();
//    apexThisOne->Init(dev, devcon);
//    apexThisOne->InitParticles();
//    return true;
//}
//
//void RecalculateView()
//{
//	// create a view matrix
//    D3DXMatrixLookAtLH(&mMatView,
//						&D3DXVECTOR3(mCam.x, mCam.y, mCam.z),    // the camera position
//                       &D3DXVECTOR3(0.0f, 0.5f, 0.0f),    // the look-at position
//                       &D3DXVECTOR3(0.0f, 1.0f, 0.0f));   // the up direction
//}