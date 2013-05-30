
#pragma once

#ifndef SYSTEM_H
#define SYSTEM_H

// include the basic windows header files and the Direct3D header files
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>
#include "GameTimer.h"
#include "Camera.h"
#include "GeometryGenerator.h"
#include "Vertices.h"
#include "RenderManager.h"
#include "apex.h"
#include <Xinput.h>
#include "MathHelper.h"


#include <iostream>
#include <fstream>
using namespace std;

//// include the Direct3D Library file
//#pragma comment (lib, "d3d11.lib")
//#pragma comment (lib, "d3dx11.lib")
//#pragma comment (lib, "d3dx10.lib")
//#pragma comment (lib, "XInput.lib")

// define the screen resolution
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

class System
{
public:
    System();
    System(HINSTANCE hInstance, int nCmdShow);
    ~System();

    int init();
    int run();
    int initd3d();



    LRESULT msgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // various buffer structs
    struct VERTEX{FLOAT X, Y, Z; D3DXCOLOR Color;};
    struct PERFRAME{D3DXCOLOR Color; FLOAT X, Y, Z;};

    // function prototypes
    void RenderFrame(float dt);                 // renders a single frame
    void CleanD3D(void);                    // closes Direct3D and releases memory
    int InitPipeline(void);                 // loads and prepares the shaders

    void SwitchScene(int scene);

    // call backs
    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);
    void UpdateCamera(float dt);
    void OnResize();
    void fpsCalc();
    

    // Random Helper
    float randomf(float low, float high);
private:
    // global declarations
    IDXGISwapChain      *swapchain;         // the pointer to the swap chain interface
    ID3D11Device        *dev;               // the pointer to our Direct3D device interface
    ID3D11DeviceContext *devcon;            // the pointer to our Direct3D device context
    D3D11_VIEWPORT mViewport;

    bool				mFlyMode;
    int					mFovFlag;

    GameTimer			mTimer;
    GameTimer			newTimer;
    float               initializationTime;

    HWND                hWnd;               // The main window
    Camera              *mCam;				// the camera
    Apex*               mApex;
    

    RenderManager       *rendManager; 
    POINT               mLastMousePos;		// the last mouse position

    GeometryGenerator   *geoGen;			// the pointer to the geometry generator

    HRESULT             hr;                 // Error checking

    int					mClientWidth;
    int					mClientHeight;

    CurrentScene		mCurrentScene;

    float				mSpeedScalar;

    bool				mAppPaused;
    bool				mMinimized;
    bool				mMaximized;
    bool				mResizing;
    bool                mInitialized;
    bool				boolRun;
    bool				switchScene;
};
#endif