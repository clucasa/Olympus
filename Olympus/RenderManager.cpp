#include "RenderManager.h"
#include "GeometryGenerator.h"

RenderManager::RenderManager(ID3D11DeviceContext *devcon, 
							 ID3D11Device *dev, 
							 IDXGISwapChain *swapchain,
							 Camera *cam) :
mDevcon(devcon), mDev(dev), mSwapchain(swapchain), mCam(cam)
{
	ID3D11Texture2D *pBackBuffer;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // use the back buffer address to create the render target
    dev->CreateRenderTargetView(pBackBuffer, NULL, &mBackbuffer);
    pBackBuffer->Release();

	GeometryGenerator *geoGen = new GeometryGenerator();

	mSkyBox = new SkyBox(mDevcon, mDev, geoGen);

	renderables.push_back(mSkyBox);

	free(geoGen);
    // set the render target as the back buffer
    devcon->OMSetRenderTargets(1, &mBackbuffer, NULL);

	
    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 64;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    mDev->CreateBuffer(&bd, NULL, &sceneCBuffer);

	//CREATE POST PROCESS RTV
	ID3D11Texture2D* pBuffer;
	mSwapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBuffer );
	

	mDev->CreateRenderTargetView( pBuffer, NULL, &mPostProcessRTV );
	pBuffer->Release();



}

void RenderManager::Render()
{
	mDevcon->VSSetConstantBuffers(0, 1, &sceneCBuffer);
	mDevcon->UpdateSubresource(sceneCBuffer, 0, 0, mCam->ViewProj().m , 0, 0);
	Render(0);
}

void RenderManager::Render(int renderType)
{
	// clear the back buffer to a deep blue
    mDevcon->ClearRenderTargetView(mBackbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));


	for(int i = 0; i < renderables.size(); i++)
	{
		renderables[i]->Render(sceneCBuffer, mCam, renderType);
	}

}

void RenderToTarget(enum renderTargets target)
{
	switch(target)
	{
	case backbuffer:

		break;
	}
}